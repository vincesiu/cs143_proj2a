/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

#define DEBUG false

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
    treeHeight = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    RC __ret = -1; //Used for return cord
    char * buffer[PageFile::PAGE_SIZE]; //used for writing in and out the index metadata

    if ( mode == 'r' ) {
        //Will open the file. Errors thrown if the file does not have metadata in page 0
        //  or if the file cannot be opened. Otherwise, it will memcpy the metadata into
        //  member variables

        this->mode = mode;

        if (this->pf.open(indexname, mode) != 0) {
            return RC_FILE_OPEN_FAILED;
        }

        if (this->pf.endPid() == 0) {
            return RC_FILE_OPEN_FAILED;
        }

        this->pf.read(0, buffer);

        memcpy((void *) &(this->rootPid), buffer, sizeof(int));
        memcpy((void *) &(this->treeHeight), ((int *) buffer) + 1, sizeof(int));

        __ret = 0;

    } else if ( mode == 'w' ) {
        //Will open the file. Errors thrown if the file cannot be opened. If index metadata
        //  exists, it will memcpy it into member variables. Otherwise, it will initialize
        //  them manually.

        this->mode = mode;

        if (this->pf.open(indexname, mode) != 0) {
            return RC_FILE_OPEN_FAILED;
        }

        if (this->pf.endPid() == 0) {
            //Initializing data for metadata page
            *((int *) buffer) = 1;
            *((int *) buffer + 1) = 1;
            this->pf.write(0, buffer);
            this->treeHeight = 1;
            this->rootPid = 1;

            //Setting up the root node
            memset(buffer, 0, PageFile::PAGE_SIZE);
            this->pf.write(1, buffer);
        } else {
            //Reading in data from metada page
            this->pf.read(0, buffer);
            memcpy((void *) &(this->rootPid), buffer, sizeof(int));
            memcpy((void *) &(this->treeHeight), ((int *) buffer) + 1, sizeof(int));
        }

        __ret = 0;

    } else {
        //Invalid mode passed in
        __ret = RC_INVALID_FILE_MODE;
    }

    return __ret;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    //Saving metadata
    char buffer[PageFile::PAGE_SIZE];
    *((int *) buffer) = this->rootPid;
    *((int *) buffer + 1) = this->treeHeight;
    this->pf.write(0, buffer);

    if (this->pf.close() != 0) {
        return RC_FILE_CLOSE_FAILED;
    }

    rootPid = -1;
    treeHeight = 0;

    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{

    BTNonLeafNode node;
    PageId rootPid = this->getRootPid();

    PageId siblingPid = rootPid;
    int siblingKey;

    RC ret = this->insertHelper(key, rid, 1, rootPid, siblingPid, siblingKey);

    //Handles updating of rootPid
    if (siblingPid != rootPid) {
        if (DEBUG) printf("Updating Root\n");
        this->rootPid = this->pf.endPid();
        this->treeHeight = this->treeHeight + 1;
        node.initializeRoot(rootPid, siblingKey, siblingPid);
        node.write(this->rootPid, this->pf);
    }

    return ret;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    BTNonLeafNode nonLeafNode;
    BTLeafNode leafNode;

    int currentLevel = 1;
    PageId pid = this->getRootPid();
    RC ret;

    //Traversing down to leaf node
    while(currentLevel < this->getTreeHeight()) {
        ret = nonLeafNode.read(pid, this->pf);
        if (ret != 0) {
            if (DEBUG) { printf("INDEX LOCATE DESCENT FAILED DURING NODE READ"); }
            return ret;
        }
        ret = nonLeafNode.locateChildPtr(searchKey, pid);
        if (ret != 0) {
            if (DEBUG) { printf("INDEX LOCATE DESCENT FAILED DURING NODE LOCATE"); }
            return ret;
        }
        currentLevel++;
    }

    //Getting value
    ret = leafNode.read(pid, this->pf);
    if (ret != 0) {
        if (DEBUG) { printf("INDEX LOCATE DESCENT FAILED DURING LEAF NODE READ"); }
        return ret;
    }

    cursor.pid = pid;
    ret = leafNode.locate(searchKey, cursor.eid);

    return ret;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 * Error modes:
 *  RC_INVALID_CURSOR: either pid or eid is not valid
 *  RC_END_OF_TREE: returned last element of tree
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    BTLeafNode node;
    //I will signal a pid of 0 as the end of tree. This is because it's
    //the default nextNodePtr when constructing a new leaf node, and I 
    //am too lazy to change it
    if (cursor.pid == 0) {
        return RC_END_OF_TREE;
    }

    if (node.read(cursor.pid, this->pf) != 0) {
        return RC_INVALID_CURSOR;
    }

    if (node.readEntry(cursor.eid, key, rid) != 0) {
        cursor.eid = 0;
        cursor.pid = node.getNextNodePtr();

        return readForward(cursor, key, rid);
    }

    //Setting next cursor
    cursor.eid++;
    /*
    if (cursor.eid < node.getKeyCount() - 1) {
        cursor.eid++;
    } else {
        cursor.eid = 0;
        cursor.pid = node.getNextNodePtr();
    }
    */

    return 0;
}


//key: search key
//rid: record ID to insert
//treeLevel: level at the tree that we are currently at
//pid: provided pid of current node we are examining
//retPid: return pid, pid != ret iff we insert and split
//retKey: changed iff insert and split
//This function is SOOO GNARLY. I'll try to fix it, but it's probably not gonna happen
RC BTreeIndex::insertHelper(int key, const RecordId& rid, int treeLevel, PageId pid, PageId& retPid, int& retKey) {
    BTLeafNode leafNode;
    BTLeafNode siblingLeaf;
    BTNonLeafNode nonLeafNode;
    BTNonLeafNode siblingNonLeaf;

    RC ret;
    int childSiblingKey;
    PageId childSiblingPid;
    PageId childPid; //this is pid of the node below this one, if this is a nonleaf
        //and we're trying to find the leaf

    if (treeLevel == this->getTreeHeight()) {
        //Inserting into leaf node


        leafNode.read(pid, this->pf);
        if (DEBUG) printf("INDEX INSERT: LEAF NEXT PID OF %d\n", leafNode.getNextNodePtr());
        ret = leafNode.insert(key, rid);
        if (ret == RC_NODE_FULL) {
            //Handling a full leaf node, use insertAndSplit
            if (DEBUG) printf("INDEX INSERT: LEAF NEXT PID OF %d\n", leafNode.getNextNodePtr());
            ret = leafNode.insertAndSplit(key, rid, siblingLeaf, retKey);
            if (DEBUG) printf("INDEX INSERT: LEAF NEXT PID OF %d\n", leafNode.getNextNodePtr());
            retPid = this->pf.endPid();
            if (DEBUG) printf("INDEX INSERT: LEAF NEXT PID OF %d\n", leafNode.getNextNodePtr());
            leafNode.setNextNodePtr(retPid);
            siblingLeaf.write(retPid, this->pf);
            if (DEBUG) printf("INDEX INSERT: SPLIT AT LEAF, ORIGINAL NEXT PID OF %d\n", leafNode.getNextNodePtr());
            if (DEBUG) printf("INDEX INSERT: SPLIT AT LEAF, SIBLING NEXT PID OF %d\n", siblingLeaf.getNextNodePtr());
        }
        if (ret != 0) {
            if (DEBUG) { printf("ERROR IN INSERT HELPER DURING INSERTION\n"); }
            return ret;
        }
        if (DEBUG) printf("INSERT AT LEAF, ORIGINAL PID OF %d\n", leafNode.getNextNodePtr());
        ret = leafNode.write(pid, this->pf);
        if (ret != 0) {
            if (DEBUG) { printf("ERROR IN INSERT HELPER DURING WRITING\n"); }
            return ret;
        }
    } else {
        //Traverse down tree
        nonLeafNode.read(pid, this->pf);
        nonLeafNode.locateChildPtr(key, childPid);
        childSiblingPid = childPid;
        ret = insertHelper(key, rid, treeLevel + 1, childPid, childSiblingPid, childSiblingKey);

        //Handling an insertAndSplit lower in the tree
        if (childPid != childSiblingPid) {
            //Insert!!!
            ret = nonLeafNode.insert(childSiblingKey, childSiblingPid);
            if (DEBUG) printf("SPLIT AT %d\n", treeLevel);

            if (ret == RC_NODE_FULL) {
                if (DEBUG) printf("MEGA SPLIT at %d!\n", treeLevel);
                //Split!!!
                nonLeafNode.insertAndSplit(childSiblingKey, childSiblingPid, siblingNonLeaf, retKey);
                retPid = this->pf.endPid();
                siblingNonLeaf.write(retPid, this->pf);
            }

            nonLeafNode.write(pid, this->pf);
        }
    }


    return 0;
}

RC BTreeIndex::getFirstElement(IndexCursor& cursor) {
    BTNonLeafNode nonLeafNode;
    PageId pid = this->getRootPid();
    int currentLevel = 1;

    while(currentLevel < this->getTreeHeight()) {
        nonLeafNode.read(pid, this->pf);
        currentLevel++;
        if (nonLeafNode.getFirstPage(pid) != 0) {
            if (DEBUG) { printf("ERROR IN DEBUGPRINTOUT WHERE EMPTY NONLEAF NODE REACHED\n"); }
            return -1;
        }
    }

    cursor.pid = pid;
    cursor.eid = 0;

    return 0;
}

//Debugging function
////////////////////////////////
void BTreeIndex::debugPrintout() {

    int currentLevel = 1;
    PageId pid = this->getRootPid();
    RecordId rid;
    int key;
    BTNonLeafNode nonLeafNode;
    BTLeafNode leafNode;


    while(currentLevel < this->getTreeHeight()) {
        nonLeafNode.read(pid, this->pf);
        currentLevel++;
        if (nonLeafNode.getFirstPage(pid) != 0) {
            if (DEBUG) { printf("ERROR IN DEBUGPRINTOUT WHERE EMPTY NONLEAF NODE REACHED\n"); }
            return;
        }
    }

    printf("----Start Printout-----\n");

    IndexCursor cursor;
    cursor.pid = pid;
    cursor.eid = 0;

    while (this->readForward(cursor, key, rid) == 0) {
        printf("------------\n");
        printf("Current key:      %d\n", key);
        printf("Next cursor page: %d\n", cursor.pid);
        printf("Next cursor eid:  %d\n", cursor.eid);
    }

    printf("----End   Printout-----\n");
}


//GETTERS
////////////////////////////////
int BTreeIndex::getRootPid() {
    return this->rootPid;
}

int BTreeIndex::getTreeHeight() {
    return this->treeHeight;
}
