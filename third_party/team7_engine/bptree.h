#ifndef BPTREE_H
#define BPTREE_H

#define BPTREE_ORDER 4
#define BPTREE_MAX_KEYS (BPTREE_ORDER - 1)

typedef struct BPTreeNode {
    int is_leaf;
    int num_keys;
    int keys[BPTREE_MAX_KEYS];
    void *values[BPTREE_MAX_KEYS];
    struct BPTreeNode *children[BPTREE_ORDER];
    struct BPTreeNode *parent;
    struct BPTreeNode *next;
} BPTreeNode;

typedef struct BPTree {
    BPTreeNode *root;
} BPTree;

BPTree *bptree_create(void);
void bptree_destroy(BPTree *tree);
int bptree_insert(BPTree *tree, int key, void *value);
void *bptree_search(BPTree *tree, int key);

#endif /* BPTREE_H */
