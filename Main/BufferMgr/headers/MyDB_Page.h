// MyDB_Page.h
#ifndef MY_DB_PAGE_H
#define MY_DB_PAGE_H

#include "MyDB_Table.h"
#include <memory>

class MyDB_Page {
public:
    MyDB_Page(MyDB_TablePtr table, long pageNum, char* data, size_t pageSize, int fd, bool isTemp = false);
    ~MyDB_Page();

    void* getBytes();
    void writeToDisk();
    bool isDirty() const;
    void setDirty(bool dirty);
    bool isPinned() const;
    void pin();
    void unpin();
    MyDB_TablePtr getTable() const;
    long getPageNum() const;
    void readFromDisk();
    bool isTemporary() const;

private:
    MyDB_TablePtr table;
    long pageNum;
    char* data;
    size_t pageSize;
    int fd;
    bool dirty;
    int pinCount;
    bool isTemp;
    std::vector<char> tempData;
};

#endif