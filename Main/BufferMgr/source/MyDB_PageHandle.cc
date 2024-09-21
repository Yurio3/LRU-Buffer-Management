
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"
#include "MyDB_Page.h"
#include <iostream>

void* MyDB_PageHandleBase::getBytes() {
    return myPage->getBytes();
}

void MyDB_PageHandleBase::wroteBytes() {
    //cout << (myPage->isTemporary() ? "Temp" : "Regular") << " page " << myPage->getPageNum() << " marked dirty" << endl;
    myPage->setDirty(true);
}

MyDB_PageHandleBase::~MyDB_PageHandleBase() {
    if (myPage->isPinned()) {
        myPage->unpin();
    }
}

MyDB_PageHandleBase::MyDB_PageHandleBase(std::shared_ptr<MyDB_Page> page) : myPage(page) {
}

std::shared_ptr<MyDB_Page> MyDB_PageHandleBase::getPage() {
    return myPage;
}

#endif

