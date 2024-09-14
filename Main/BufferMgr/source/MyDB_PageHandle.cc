
#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_PageHandle.h"
#include "MyDB_Page.h"

//#include "../../Catalog/headers/MyDB_Table.h"

void *MyDB_PageHandleBase :: getBytes () {
	//return nullptr;
	return myPage->getBytes();
}

void MyDB_PageHandleBase :: wroteBytes () {
	myPage->setDirty(true);
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
	if (myPage->isPinned()) {
        myPage->unpin();
    }
}

MyDB_PageHandleBase :: MyDB_PageHandleBase(shared_ptr<Page> page) : myPage(page) {
}

shared_ptr<Page> MyDB_PageHandleBase :: getPage() {
    return myPage;
}


#endif

