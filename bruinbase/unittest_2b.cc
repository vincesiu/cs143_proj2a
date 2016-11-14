#include <cstdio>
#include <cstdlib>

#include "BTreeNode.h"

int main(int argc, char** argv) {
    printf("hello world");

    BTLeafNode * leaf = (BTLeafNode *) malloc(sizeof(BTLeafNode));
    BTNonLeafNode * nonleaf = (BTNonLeafNode *) malloc(sizeof(BTNonLeafNode));
}
