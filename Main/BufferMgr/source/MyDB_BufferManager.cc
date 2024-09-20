#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"
#include "MyDB_PageHandle.h"
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <memory>
#include <iostream>

using namespace std;

MyDB_PageHandle MyDB_BufferManager::getPage(MyDB_TablePtr whichTable, long i) {
    return make_shared<MyDB_PageHandleBase>(getPageInternal(whichTable, i, false));
}

MyDB_PageHandle MyDB_BufferManager::getPage() {
    cout << "Creating temporary page" << endl;
    static long tempPageCounter = 0;
    return make_shared<MyDB_PageHandleBase>(getPageInternal(nullptr, tempPageCounter++, false));
}

MyDB_PageHandle MyDB_BufferManager::getPinnedPage(MyDB_TablePtr whichTable, long i) {
    return make_shared<MyDB_PageHandleBase>(getPageInternal(whichTable, i, true));
}

MyDB_PageHandle MyDB_BufferManager::getPinnedPage() {
    return getPinnedPage(nullptr, -1);  // Use nullptr for anonymous pages
}

void MyDB_BufferManager::unpin(MyDB_PageHandle unpinMe) {
    dynamic_pointer_cast<MyDB_PageHandleBase>(unpinMe)->getPage()->unpin();
}

MyDB_BufferManager::MyDB_BufferManager(size_t pageSize, size_t numPages, string tempFile)
    : pageSize(pageSize), numPages(numPages), tempFile(tempFile) {
    for (size_t i = 0; i < numPages; i++) {
        bufferPool.push_back(new char[pageSize]);
    }
    tempFileFD = open(tempFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
}

MyDB_BufferManager::~MyDB_BufferManager() {
    for (auto& pair : pageMap) {
        if (pair.second->isDirty()) {
            pair.second->writeToDisk();
        }
    }
    for (char* buffer : bufferPool) {
        delete[] buffer;
    }
    close(tempFileFD);
    remove(tempFile.c_str());
}

shared_ptr<MyDB_Page> MyDB_BufferManager::getPageInternal(MyDB_TablePtr whichTable, long i, bool pinned) {
    auto key = make_pair(whichTable, i);
    if (pageMap.find(key) != pageMap.end()) {
        auto page = pageMap[key];
        if (pinned) page->pin();
        lru.touchPage(page);
        return page;
    }

    char* data;
    bool isTemp = (whichTable == nullptr);
    
    if (isTemp || bufferPool.empty()) {
        data = new char[pageSize];
    } else {
        data = bufferPool.back();
        bufferPool.pop_back();
    }

    int fd = (whichTable == nullptr) ? tempFileFD : getTableFD(whichTable);
    auto page = make_shared<MyDB_Page>(whichTable, i, data, pageSize, fd, isTemp);
    if (pinned) page->pin();
    pageMap[key] = page;
    lru.addPage(page);

    if (whichTable != nullptr) {
        page->readFromDisk();
    }

    return page;
}

void MyDB_BufferManager::evictPage() {
    auto pageToEvict = lru.evict();
    if (pageToEvict == nullptr) {
        throw std::runtime_error("No page available for eviction");
    }
    
    cout << "Evicting " << (pageToEvict->isTemporary() ? "temporary" : "regular") << " page " << pageToEvict->getPageNum() << endl;
    
    if (pageToEvict->isDirty()) {
        pageToEvict->writeToDisk();
    }
    
    if (!pageToEvict->isTemporary()) {
        bufferPool.push_back(static_cast<char*>(pageToEvict->getBytes()));
    } else {
        // For temporary pages, we don't return the buffer to the pool
        // Instead, we'll allocate a new buffer when needed
        delete[] static_cast<char*>(pageToEvict->getBytes());
    }
    
    pageMap.erase(make_pair(pageToEvict->getTable(), pageToEvict->getPageNum()));
}

int MyDB_BufferManager::getTableFD(MyDB_TablePtr whichTable) {
    // This is a simplified version. You might want to maintain a map of open file descriptors
    return open(whichTable->getStorageLoc().c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
}

#endif