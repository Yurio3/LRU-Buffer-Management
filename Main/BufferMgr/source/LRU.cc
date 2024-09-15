// LRU.cc
#include "LRU.h"
#include "MyDB_Page.h"

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
        if (!page->isPinned()) {
            lruList.pop_back();
            pageMap.erase(page);
            return page;
        }
        lruList.pop_back();
        lruList.push_front(page);
        pageMap[page] = lruList.begin();
    }
    throw std::runtime_error("No unpinned pages available for eviction");
}