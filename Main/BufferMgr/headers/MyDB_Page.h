#ifndef MY_DB_PAGE_H
#define MY_DB_PAGE_H

#include "MyDB_Table.h"
#include <memory>
class Node;

class Page {
public:
    //new
    Node* node;
    //anonymous
    Page();
    int getSlotID();
    void setSlotID(int slotid);
    void setBytes(Node *node);
    Node* getBytes();
    bool isAnonymous();
    char *getBufferAddress();
    void setBufferAddress(char *bufferAddr);
    int getHandleNum();
    void addHandleNum();
    void minuHandleNum();
    


    //non-anonymous
    Page(MyDB_TablePtr table, long pageNum, char* data, size_t pageSize, int fd);
    ~Page();

    //get data of page
    
    //write back to disk
    void writeToDisk();
    //check page is modify
    bool isDirty() const;
    void setDirty(bool dirty);

    bool isPinned() const;
    void pin();
    void unpin();

    //get table of page
    MyDB_TablePtr getTable() const;
    //get page number in table
    long getPageNum() const;

private:
    MyDB_TablePtr table;
    long pageNum;
//    char* data;
    size_t pageSize;
    int fd; //is this bufferaddress?
    bool dirty;
    int pinCount;

    //new
    int slotID = 0;
    bool isAnon;
    int handleNum = 0; 
    Node* node;
    //追踪有多少pagehandle指向它，老师说的我没太懂，basic setup:  PageHandle → PageHandleBase → Page
    char* bufferAddress = nullptr;
    
};

#endif