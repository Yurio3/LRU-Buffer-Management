// LRU.cc
#include "LRU.h"
#include "MyDB_Page.h"
#include <stdexcept>
#include <iostream>


void LRU::addPage(std::shared_ptr<MyDB_Page> page) {
    lruList.push_front(page);
    pageMap[page] = lruList.begin();
}

void LRU::touchPage(std::shared_ptr<MyDB_Page> page) {
    if (pageMap.find(page) != pageMap.end()) {
        lruList.erase(pageMap[page]);
        lruList.push_front(page);
        pageMap[page] = lruList.begin();
    }
}

std::shared_ptr<MyDB_Page> LRU::evict() {
    while (!lruList.empty()) {
        auto page = lruList.back();
        lruList.pop_back();
        pageMap.erase(page);
        
        if (!page->isPinned()) {
            cout << "LRU evicting " << (page->isTemporary() ? "temporary" : "regular") << " page " << page->getPageNum() << endl;
            return page;
        }
        
        // If the page is pinned, move it to the front
        lruList.push_front(page);
        pageMap[page] = lruList.begin();
    }
    
    return nullptr;
}