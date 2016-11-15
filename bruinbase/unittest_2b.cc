#include <cstdio>
#include <cstdlib>

#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeNode.h"
#include "RecordFile.h"

int main(int argc, char** argv) {
	RecordId rid;
    rid.pid = 11;
    rid.sid = 7;

    BTLeafNode splitTestLeaf;
    BTLeafNode afterSplitTestLeaf;

    for (int i = 1; i <= 70; i++) {
    	splitTestLeaf.insert(5*i, rid);
    }

    int siblingKey = 0;
    splitTestLeaf.insertAndSplit(33, rid, afterSplitTestLeaf, siblingKey);
    printf("Printing split_0 -- look for tuple with key=33\n");
    splitTestLeaf.printNode();

    printf("Printing split_1 starting from %i \n", siblingKey);
    afterSplitTestLeaf.printNode();

    splitTestLeaf.insert(11, rid);
    splitTestLeaf.insert(222, rid);
    printf("Printing split_0 after inserting 11 and 222\n");
    splitTestLeaf.printNode();
}

// TEST BASIC INSERT -- LEAF NODE

// int main(int argc, char** argv) {
//     BTLeafNode testLeaf;
    
//     RecordId rid;
//     rid.pid = 11;
//     rid.sid = 7;

//     testLeaf.printNode();
//     testLeaf.insert(10, rid);
//     testLeaf.insert(20, rid);
//     testLeaf.printNode();
//     testLeaf.insert(15, rid);
//     testLeaf.printNode();   
// }

//

// TEST INSERT & SPLIT -- LEAF NODE

// int main(int argc, char** argv) {
//     RecordId rid;
//     rid.pid = 11;
//     rid.sid = 7;

//     BTLeafNode splitTestLeaf;
//     BTLeafNode afterSplitTestLeaf;

//     for (int i = 1; i <= 70; i++) {
//     	splitTestLeaf.insert(5*i, rid);
//     }

//     int siblingKey = 0;
//     splitTestLeaf.insertAndSplit(33, rid, afterSplitTestLeaf, siblingKey);
//     printf("Printing split_0 -- look for tuple with key=33\n");
//     splitTestLeaf.printNode();

//     printf("Printing split_1 starting from %i \n", siblingKey);
//     afterSplitTestLeaf.printNode();

//     splitTestLeaf.insert(11, rid);
//     splitTestLeaf.insert(222, rid);
//     printf("Printing split_0 after inserting 11 and 222\n");
//     splitTestLeaf.printNode();
// }