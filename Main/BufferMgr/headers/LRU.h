// LRU.h
#ifndef LRU_H
#define LRU_H

#include <list>
#include <unordered_map>
#include <memory>

class MyDB_Page;

class LRU {
public:
    void addPage(std::shared_ptr<MyDB_Page> page);
    void touchPage(std::shared_ptr<MyDB_Page> page);
    std::shared_ptr<MyDB_Page> evict();

private:
    std::list<std::shared_ptr<MyDB_Page>> lruList;
    std::unordered_map<std::shared_ptr<MyDB_Page>, std::list<std::shared_ptr<MyDB_Page>>::iterator> pageMap;
};

#endif