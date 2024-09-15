#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"
#include "MyDB_PageHandle.h"
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <memory>

using namespace std;

MyDB_PageHandle MyDB_BufferManager::getPage(MyDB_TablePtr whichTable, long i) {
    return make_shared<MyDB_PageHandleBase>(getPageInternal(whichTable, i, false));
}

MyDB_PageHandle MyDB_BufferManager::getPage() {
    return getPage(nullptr, -1);  // Use nullptr for anonymous pages
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

    if (bufferPool.empty()) {
        evictPage();
    }

    char* data = bufferPool.back();
    bufferPool.pop_back();

    int fd = (whichTable == nullptr) ? tempFileFD : getTableFD(whichTable);
    auto page = make_shared<MyDB_Page>(whichTable, i, data, pageSize, fd);
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
    if (pageToEvict->isDirty()) {
        pageToEvict->writeToDisk();
    }
    bufferPool.push_back(static_cast<char*>(pageToEvict->getBytes()));
    pageMap.erase(make_pair(pageToEvict->getTable(), pageToEvict->getPageNum()));
}

int MyDB_BufferManager::getTableFD(MyDB_TablePtr whichTable) {
    // This is a simplified version. You might want to maintain a map of open file descriptors
    return open(whichTable->getStorageLoc().c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
}

#endif