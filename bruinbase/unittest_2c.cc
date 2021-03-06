#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "BTreeIndex.h"

#define DEBUGPRINTOUT true
int main (int argc, char **argv) {
    //Random variables
    IndexCursor cursor;
    int i;
    RecordId rid;

    printf("----------------Beginning Tests:--------------------\n");

    BTreeIndex index;
    printf("Testing open and close functionality:");


    char const *fileName = "test.index";
    assert(index.open(fileName, 'f') == RC_INVALID_FILE_MODE);
    assert(index.open(fileName, 'r') == RC_FILE_OPEN_FAILED);
    assert(index.open(fileName, 'w') == 0);
    assert(index.getRootPid() == 1);
    assert(index.getTreeHeight() == 1);
    assert(index.close() == 0);
    assert(index.open(fileName, 'r') == 0);
    assert(index.getRootPid() == 1); 
    assert(index.getTreeHeight() == 1);
    assert(index.close() == 0);
    assert(index.open(fileName, 'w') == 0);
    assert(index.getRootPid() == 1); 
    assert(index.getTreeHeight() == 1);

    printf(" Good!\n");
    if (DEBUGPRINTOUT) index.debugPrintout();

    printf("Testing basic insert functionality:");

    for (i = 0; i < 70; i++) {
        rid.pid = 0;
        rid.sid = i;
        assert(index.insert(i, rid) == 0);
    }

    printf(" Good!\n");
    if (DEBUGPRINTOUT) index.debugPrintout();


    printf("Testing basic search functionality:");
    assert(index.locate(1, cursor) == 0);
    assert(cursor.pid == 1);
    assert(cursor.eid == 1);
    assert(index.locate(69, cursor) == 0);
    assert(cursor.pid == 1);
    assert(cursor.eid == 69);
    assert(index.locate(70, cursor) == RC_NO_SUCH_RECORD);
    printf(" Good!\n");

    printf("Testing advanced insert functionality (new root creation):");
    rid.pid = 0;
    rid.sid = 70;
    assert(index.insert(70, rid) == 0);
    assert(index.getRootPid() == 3);
    assert(index.getTreeHeight() == 2);
    assert(index.locate(70, cursor) == 0);
    assert(cursor.pid == 2);
    assert(cursor.eid == 34);
    assert(index.close() == 0);
    assert(index.open(fileName, 'w') == 0);
    assert(index.getRootPid() == 3); 
    assert(index.getTreeHeight() == 2);
    printf(" Good!\n");

    printf("Testing advanced insert functionality (inserting into a nonleaf):");
    for (i = 71; i < 107; i++) {
        rid.pid = 0;
        rid.sid = i;
        assert(index.insert(i, rid) == 0);
    }

    if (DEBUGPRINTOUT) index.debugPrintout();


    printf(" Good!\n");
    printf("Final test:");

    for (i = 107; i < 3500; i++) {
        rid.pid = 0;
        rid.sid = i;
        assert(index.insert(i, rid) == 0);
    }

    assert(index.close() == 0);
    assert(index.open(fileName, 'r') == 0);

    assert(index.locate(0, cursor) == 0);
    assert(index.locate(1000, cursor) == 0);
    assert(index.locate(2000, cursor) == 0);
    assert(index.locate(2999, cursor) == 0);
    assert(index.locate(3500, cursor) == RC_NO_SUCH_RECORD);

    if (DEBUGPRINTOUT) index.debugPrintout();
    printf(" Good!\n");


    printf("----------------Ending Test--------------------\n");
}
