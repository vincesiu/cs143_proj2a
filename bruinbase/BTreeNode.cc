#include "BTreeNode.h"

using namespace std;

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
    pf.read(pid, this->buffer);
    return 0; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
    pf.write(pid, this->buffer); 
    return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
    // return this->keyCount; 
    int temp;
    memcpy(&temp, buffer + (PageFile::PAGE_SIZE - sizeof(temp)), sizeof(temp));
    return temp;
}

/**
* Set the key count to n
*/
void BTLeafNode::setKeyCount(int n) {
   memcpy(buffer + (PageFile::PAGE_SIZE - sizeof(n)), (char *) &n, sizeof(n));
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
    //TODO
    
    int keyCount = this->getKeyCount();
    if (keyCount == MAXIMUM_KEY_COUNT) {
        // TODO: error -> node is full
        return -1;
    }

    int eid;
    RC returncode = BTLeafNode::locate(key, eid);

    if (returncode == 0) {
    	// TODO: error -> key already exists in node
    	return -2;
    }

    char tempbuffer[PageFile::PAGE_SIZE];
    // must subtract 12 to prevent buffer overflow
    int sizeToCopy = PageFile::PAGE_SIZE - (eid * 12) - 12;
    
    // store everything between eid and end of buffer to temp array
    memcpy(tempbuffer, buffer + (eid * 12), sizeToCopy);

    // store new record's key into eid of original buffer
    memcpy(buffer + (eid * 12), (char *) &key, sizeof(key));

    // store new record's pointers info into eid + 4 of original buffer
    memcpy(buffer + (eid * 12) + 4, (char *) rid.pid, sizeof(rid.pid));
    memcpy(buffer + (eid * 12) + 8, (char *) rid.sid, sizeof(rid.sid));

    // move temp buffer back to original buffer but in eid + 1
    memcpy(buffer + ((eid + 1) * 12), tempbuffer, sizeToCopy);

    // adjust key count
    setKeyCount(keyCount + 1);
    
    return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
    //TODO
    return 0; 
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
    int keyCount = this->getKeyCount();
    int curKey;
    int slot = -1;

    RC ret = RC_NO_SUCH_RECORD;
    for (int i = 0; i < keyCount; i++) {
        // key is 4 bytes, pointer is 8 bytes
        // when eid is i, access i * 12 in buffer array
        memcpy(&curKey, buffer + (i * 12), sizeof(curKey));
        if (curKey == searchKey) {
            slot = i;
            ret = 0;
            break;
        } else if (curKey > searchKey) {
        	slot = i;
        	break;
        }
    }

    if (slot == -1) eid = keyCount;
    else eid = slot;

    return ret;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
    int keyCount = this->getKeyCount();
    if (eid > keyCount) return -1;

    memcpy(&key, buffer + (eid * 12), sizeof(key));
    memcpy(&rid.pid, buffer + (eid * 12) + 4, sizeof(rid.pid));
    memcpy(&rid.sid, buffer + (eid * 12) + 8, sizeof(rid.sid));

    return 0; 
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
    PageId temp;
    memcpy(&temp, buffer + (PageFile::PAGE_SIZE - ( 2 * sizeof(int) ) ), sizeof(temp));
    return temp; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
    memcpy(buffer + (PageFile::PAGE_SIZE - (2 * sizeof(pid))), (char *) &pid, sizeof(pid));
    return 0; 
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
    pf.read(pid, this->buffer);
    return 0; 
}

    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
    pf.write(pid, this->buffer);
    return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
    // return this->keyCount; 
    int temp;
    memcpy(&temp, buffer + (PageFile::PAGE_SIZE - sizeof(temp)), sizeof(temp));
    return temp;
}

/**
* Set the key count to n
*/
void BTNonLeafNode::setKeyCount(int n) {
   memcpy(buffer + (PageFile::PAGE_SIZE - sizeof(n)), (char *) &n, sizeof(n));
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ 
    if (this->keyCount == MAXIMUM_KEY_COUNT) {
        //Error message
        return -1;
    }
    return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ 
    //TODO
    return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 
    //TODO
    return 0; 
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
    memset(this->buffer, PageFile::PAGE_SIZE, sizeof(char)); //reset buffer

    //set things
    //////////////////////
    int address = 0;
    memcpy(this->buffer, (char *) &pid1, address);

    address += sizeof(pid1);
    memcpy(this->buffer, (char *) &key, address);

    address += sizeof(key);
    memcpy(this->buffer, (char *) &pid2, address);

    this->setKeyCount(1); //set keycount to 1
    
    return 0; 
}
