
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include <string>

using namespace std;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr whichTable, long i) {
	//return nullptr;
	return make_shared<MyDB_PageHandleBase>(findOrCreatePage(whichTable, i, false));		
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
	// nullptr;		
	return make_shared<MyDB_PageHandleBase>(findOrCreatePage(nullptr, anonymousCounter++, false));
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr whichTable, long i) {
	//return nullptr;	
	return make_shared<MyDB_PageHandleBase>(findOrCreatePage(whichTable, i, true));	
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
	//return nullptr;	
	return make_shared<MyDB_PageHandleBase>(findOrCreatePage(nullptr, anonymousCounter++, true));	
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
	shared_ptr<MyDB_Page> page = static_pointer_cast<MyDB_PageHandleBase>(unpinMe)->getPage();
    page->unpin();
    if (!page->isPinned()) {
        addToLRU(page);
    }
}

// MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
// 	: pageSize(pageSize), numPages(numPages), tempFile(tempFile), anonymousCounter(0) {
//     bufferPool = new char[pageSize * numPages];
//     tempFileFD = open(tempFile.c_str(), O_RDWR | O_CREAT | O_FSYNC, 0666);
//     if (tempFileFD == -1) {
//         cerr << "Failed to open temporary file" << endl;
//         exit(1);
//     }
// }

// MyDB_BufferManager :: ~MyDB_BufferManager () {
// }

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) 
    : pageSize(pageSize), numPages(numPages), tempFile(tempFile), anonymousCounter(0) {
    bufferPool = new char[pageSize * numPages];
    tempFileFD = open(tempFile.c_str(), O_RDWR | O_CREAT | O_FSYNC, 0666);
    if (tempFileFD == -1) {
        cerr << "Failed to open temporary file" << endl;
        exit(1);
    }
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    for (auto& pair : pageTable) {
        if (pair.second->isDirty()) {
            pair.second->writeToDisk();
        }
    }
    close(tempFileFD);
    unlink(tempFile.c_str());
    delete[] bufferPool;
}

shared_ptr<MyDB_Page> MyDB_BufferManager::findOrCreatePage(MyDB_TablePtr whichTable, long i, bool isPinned) {
    auto key = make_pair(whichTable, i);
    auto it = pageTable.find(key);

    if (it != pageTable.end()) {
        auto page = it->second;
        if (isPinned) {
            page->pin();
        }
        addToLRU(page);
        return page;
    }

    if (pageTable.size() >= numPages) {
        evictPage();
    }

    char* pageData = bufferPool + (pageTable.size() * pageSize);
    auto newPage = make_shared<MyDB_Page>(whichTable, i, pageData, pageSize, tempFileFD);
    
    if (isPinned) {
        newPage->pin();
    } else {
        addToLRU(newPage);
    }

    pageTable[key] = newPage;
    return newPage;
}

void MyDB_BufferManager::evictPage() {
    for (auto it = lruList.rbegin(); it != lruList.rend(); ++it) {
        if (!(*it)->isPinned()) {
            if ((*it)->isDirty()) {
                (*it)->writeToDisk();
            }
            pageTable.erase(make_pair((*it)->getTable(), (*it)->getPageNum()));
            lruList.erase(next(it).base());
            return;
        }
    }
    cerr << "No unpinned pages available for eviction" << endl;
    exit(1);
}

void MyDB_BufferManager::addToLRU(shared_ptr<MyDB_Page> page) {
    lruList.remove(page);
    lruList.push_front(page);
}
	
#endif


