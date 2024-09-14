#include "MyDB_Page.h"
#include <unistd.h>

MyDB_Page::Page(MyDB_TablePtr table, long pageNum, char* data, size_t pageSize, int fd)
    : table(table), pageNum(pageNum), data(data), pageSize(pageSize), fd(fd), dirty(false), pinCount(0) {}

MyDB_Page::~Page() {
    if (dirty) {
        writeToDisk();
    }
}

void* MyDB_Page::getBytes() {
    return data;
}

void MyDB_Page::writeToDisk() {
    if (table) {
        lseek(fd, pageNum * pageSize, SEEK_SET);
        write(fd, data, pageSize);
    }
    dirty = false;
}

bool MyDB_Page::isDirty() const {
    return dirty;
}

void MyDB_Page::setDirty(bool dirty) {
    this->dirty = dirty;
}

bool MyDB_Page::isPinned() const {
    return pinCount > 0;
}

void MyDB_Page::pin() {
    pinCount++;
}

void MyDB_Page::unpin() {
    if (pinCount > 0) {
        pinCount--;
    }
}

MyDB_TablePtr MyDB_Page::getTable() const {
    return table;
}

long MyDB_Page::getPageNum() const {
    return pageNum;
}