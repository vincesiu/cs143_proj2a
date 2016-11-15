#include <cstdio>
#include <cstdlib>

#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeNode.h"
#include "RecordFile.h"

// unit test declarations
void testBasicLeaf();
void testInsertSplitLeaf();
void testBasicNonLeaf();
void testInsertSplitNonLeaf();


int main(int argc, char** argv) {
	testBasicLeaf();
	testInsertSplitLeaf();
	testBasicNonLeaf();
	testInsertSplitNonLeaf();
}


//

// TEST INSERT & SPLIT -- NON LEAF NODE

void testInsertSplitNonLeaf() {
	PageId p1 = 7;
	PageId p2 = 11;
    
    BTNonLeafNode testNonLeaf;
    testNonLeaf.initializeRoot(p1,3,p2);
    
    for (int i = 0; i < 70; i++) {
    	if (testNonLeaf.insert((13 + 7*i), (14+i)) < 0) 
    		printf("NonLeafNode at max size\n");
    }

    printf("Node prior to split\n");
    testNonLeaf.printNode();

    BTNonLeafNode testPostSplit;
    int midKey = -1;

    testNonLeaf.insertAndSplit(100,999,testPostSplit,midKey);
    printf("Left node after split, insert key=100\n");
    testNonLeaf.printNode();
	printf("Node split on key=%i\n", midKey);
    printf("Right node after split\n");
    testPostSplit.printNode();
}

//

// TEST INITIALIZE, INSERT & BASIC FUNCTIONS -- NON LEAF NODE

void testBasicNonLeaf() {
	PageId p1 = 7;
	PageId p2 = 11;
    
    // test initializeRoot
    BTNonLeafNode testNonLeaf;
    testNonLeaf.initializeRoot(p1,33,p2);
    testNonLeaf.printNode();

    // test insert
    testNonLeaf.insert(11,1);
    testNonLeaf.insert(77,13);
    testNonLeaf.insert(99,17);
    testNonLeaf.insert(55,19);
    testNonLeaf.printNode();

    testNonLeaf.locateChildPtr(5, p2);
    printf("child with key=%i found at pid=%i\n", 5, p2);
    testNonLeaf.locateChildPtr(55, p2);
    printf("child with key=%i found at pid=%i\n", 55, p2);
    testNonLeaf.locateChildPtr(108, p2);
    printf("child with key=%i found at pid=%i\n", 108, p2);
}

//

// TEST INSERT & BASIC FUNCTIONS -- LEAF NODE

void testBasicLeaf() {
    BTLeafNode testLeaf;
    
    RecordId rid;
    rid.pid = 11;
    rid.sid = 7;

    testLeaf.printNode();
    testLeaf.insert(10, rid);
    testLeaf.insert(20, rid);
    testLeaf.printNode();
    testLeaf.insert(15, rid);
    testLeaf.printNode();

    int eid;
    testLeaf.locate(15, eid);
    int key;
    testLeaf.readEntry(eid,key,rid);
    printf("key 15 located at eid=%i, readEntry(eid) found key=%i\n",eid,key);

    testLeaf.setNextNodePtr(25);
    printf("nextnodeptr [should be 25]: %i\n",testLeaf.getNextNodePtr());
}

//

// TEST INSERT & SPLIT -- LEAF NODE

void testInsertSplitLeaf() {
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