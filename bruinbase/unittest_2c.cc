#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "BTreeIndex.h"

int main (int argc, char **argv) {

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
    index.debugPrintout();

    printf("Testing insert functionality:");

    int i;
    RecordId rid;
    for (i = 0; i < 70; i++) {
        rid.pid = 0;
        rid.sid = i;
        assert(index.insert(i, rid) == 0);
    }

    printf(" Good!\n");
    index.debugPrintout();


    printf("----------------Ending Test--------------------\n");
}
