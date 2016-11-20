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
            //Initialization
            *((int *) buffer) = 1;
            *((int *) buffer + 1) = 1;
            this->pf.write(0, buffer);
            this->treeHeight = 1;
            this->rootPid = 1;

            //Setting first root node
            memset(buffer, 0, PageFile::PAGE_SIZE);
            this->pf.write(1, buffer);
        } else {
            this->pf.read(0, buffer);
            memcpy((void *) &(this->treeHeight), buffer, sizeof(int));
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
    return 0;
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
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    return 0;
}


PageId BTreeIndex::insertHelper(int key, const RecordId& rid, int treeLevel) {
    /*
PageId BTreeIndex::insertHelper(int key, const RecordId& rid, int treeLevel, PageId) {
    BTLeafNode leafNode;
    BTNonLeafNode nonLeafNode;

    if (index.getTreeHeight() == treeLevel) {
        leafNode.read(
    }
    */
    return 0;
}
//Debugging function
////////////////////////////////


//Some extra setters for me
////////////////////////////////
int BTreeIndex::getRootPid() {
    return this->rootPid;
}

int BTreeIndex::getTreeHeight() {
    return this->treeHeight;
}
