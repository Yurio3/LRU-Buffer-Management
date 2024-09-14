#include "MyDB_Page.h"
#include <unistd.h>


Page::Page(MyDB_TablePtr table, long pageNum, Node* node, size_t pageSize, int fd)
    : table(table), pageNum(pageNum), node(node), pageSize(pageSize), fd(fd), dirty(false), pinCount(0) {}

Page::~Page() {
    if (dirty) {
        writeToDisk();
    }
}

Node* Page::getBytes() {
    return node;
}

void Page::writeToDisk() {
    if (table) {
        lseek(fd, pageNum * pageSize, SEEK_SET);
        write(fd, node, pageSize);
    }
    dirty = false;
}

bool Page::isDirty() const {
    return dirty;
}

void Page::setDirty(bool dirty) {
    this->dirty = dirty;
}

bool Page::isPinned() const {
    return pinCount > 0;
}

void Page::pin() {
    pinCount++;
}

void Page::unpin() {
    if (pinCount > 0) {
        pinCount--;
    }
}

MyDB_TablePtr Page::getTable() const {
    return table;
}

long Page::getPageNum() const {
    return pageNum;
}