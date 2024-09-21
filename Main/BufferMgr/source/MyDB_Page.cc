// MyDB_Page.cc
#include "MyDB_Page.h"
#include <unistd.h>
#include <iostream>
#include <cstring>

MyDB_Page::MyDB_Page(MyDB_TablePtr table, long pageNum, char* data, size_t pageSize, int fd, bool isTemp)
    : table(table), pageNum(pageNum), data(data), pageSize(pageSize), fd(fd), dirty(false), pinCount(0), isTemp(isTemp) {
        if (isTemp) {
            tempData.resize(pageSize);
            std::memcpy(tempData.data(), data, pageSize);
        }
    }

MyDB_Page::~MyDB_Page() {
    if (dirty) {
        writeToDisk();
    }
    if (isTemp) {
        // For temporary pages, we need to free the data buffer
        // since it's not managed by the buffer pool
        delete[] data;
    }
    // For non-temporary pages, we don't delete the data buffer here
    // because it's managed by the MyDB_BufferManager
}

void* MyDB_Page::getBytes() {
    //cout << (isTemp ? "Temp" : "Regular") << " page " << pageNum << " accessed" << endl;
    return isTemp ? tempData.data() : data;
}


void MyDB_Page::writeToDisk() {
     //cout << (isTemp ? "Temp" : "Regular") << " page " << pageNum 
        // << (isTemp ? " storing data" : " writing to disk") << endl;
    if (isTemp) {
        std::memcpy(tempData.data(), data, pageSize);
    } else if (table) {
        lseek(fd, pageNum * pageSize, SEEK_SET);
        write(fd, data, pageSize);
    }
    //cout << (isTemp ? "Temp" : "Regular") << " page " << pageNum << " marked clean" << endl;
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

void MyDB_Page::readFromDisk() {
    if (table) {
        lseek(fd, pageNum * pageSize, SEEK_SET);
        read(fd, data, pageSize);
    }
}

bool MyDB_Page::isTemporary() const {
    return isTemp;
}