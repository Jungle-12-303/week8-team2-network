#include "bptree.h"

#include <stdlib.h>

static BPTreeNode *bptree_create_node(int is_leaf)
{
    BPTreeNode *node;

    node = (BPTreeNode *)calloc(1, sizeof(BPTreeNode));
    if (node == NULL) {
        return NULL;
    }

    node->is_leaf = is_leaf;
    return node;
}

static void bptree_destroy_node(BPTreeNode *node)
{
    int index;

    if (node == NULL) {
        return;
    }

    if (!node->is_leaf) {
        for (index = 0; index <= node->num_keys; index++) {
            bptree_destroy_node(node->children[index]);
        }
    }

    free(node);
}

static BPTreeNode *bptree_find_leaf(BPTree *tree, int key)
{
    BPTreeNode *node;
    int child_index;

    if (tree == NULL || tree->root == NULL) {
        return NULL;
    }

    node = tree->root;
    while (!node->is_leaf) {
        child_index = 0;

        while (child_index < node->num_keys &&
            key >= node->keys[child_index]) {
            child_index++;
        }

        node = node->children[child_index];
    }

    return node;
}

static int bptree_find_insert_index(const int *keys, int num_keys, int key)
{
    int index = 0;

    while (index < num_keys && keys[index] < key) {
        index++;
    }

    return index;
}

static void bptree_insert_into_leaf(BPTreeNode *leaf, int key, void *value)
{
    int index;
    int insert_index;

    insert_index = bptree_find_insert_index(leaf->keys, leaf->num_keys, key);

    for (index = leaf->num_keys; index > insert_index; index--) {
        leaf->keys[index] = leaf->keys[index - 1];
        leaf->values[index] = leaf->values[index - 1];
    }

    leaf->keys[insert_index] = key;
    leaf->values[insert_index] = value;
    leaf->num_keys++;
}

static int bptree_insert_into_parent(BPTree *tree, BPTreeNode *left,
    int key, BPTreeNode *right);

static int bptree_split_internal(BPTree *tree, BPTreeNode *node)
{
    BPTreeNode *right;
    int total_keys;
    int promote_index;
    int promote_key;
    int index;
    int right_key_index;
    int right_child_index;

    total_keys = node->num_keys;
    promote_index = total_keys / 2;
    promote_key = node->keys[promote_index];

    right = bptree_create_node(0);
    if (right == NULL) {
        return 0;
    }

    right_key_index = 0;
    for (index = promote_index + 1; index < total_keys; index++) {
        right->keys[right_key_index++] = node->keys[index];
    }
    right->num_keys = right_key_index;

    right_child_index = 0;
    for (index = promote_index + 1; index <= total_keys; index++) {
        right->children[right_child_index] = node->children[index];
        if (right->children[right_child_index] != NULL) {
            right->children[right_child_index]->parent = right;
        }
        right_child_index++;
    }

    node->num_keys = promote_index;
    return bptree_insert_into_parent(tree, node, promote_key, right);
}

static int bptree_split_leaf(BPTree *tree, BPTreeNode *leaf)
{
    BPTreeNode *right;
    int split_index;
    int index;
    int right_index;
    int promote_key;

    split_index = (leaf->num_keys + 1) / 2;
    right = bptree_create_node(1);
    if (right == NULL) {
        return 0;
    }

    right_index = 0;
    for (index = split_index; index < leaf->num_keys; index++) {
        right->keys[right_index] = leaf->keys[index];
        right->values[right_index] = leaf->values[index];
        right_index++;
    }

    right->num_keys = right_index;
    leaf->num_keys = split_index;

    right->next = leaf->next;
    leaf->next = right;
    promote_key = right->keys[0];

    return bptree_insert_into_parent(tree, leaf, promote_key, right);
}

static int bptree_insert_into_parent(BPTree *tree, BPTreeNode *left,
    int key, BPTreeNode *right)
{
    BPTreeNode *parent;
    int insert_index;
    int index;

    parent = left->parent;
    right->parent = parent;

    if (parent == NULL) {
        parent = bptree_create_node(0);
        if (parent == NULL) {
            return 0;
        }

        parent->keys[0] = key;
        parent->children[0] = left;
        parent->children[1] = right;
        parent->num_keys = 1;
        left->parent = parent;
        right->parent = parent;
        tree->root = parent;
        return 1;
    }

    insert_index = bptree_find_insert_index(parent->keys, parent->num_keys, key);
    for (index = parent->num_keys; index > insert_index; index--) {
        parent->keys[index] = parent->keys[index - 1];
    }
    for (index = parent->num_keys + 1; index > insert_index + 1; index--) {
        parent->children[index] = parent->children[index - 1];
    }

    parent->keys[insert_index] = key;
    parent->children[insert_index + 1] = right;
    parent->num_keys++;

    if (parent->num_keys > BPTREE_MAX_KEYS) {
        return bptree_split_internal(tree, parent);
    }

    return 1;
}

BPTree *bptree_create(void)
{
    return (BPTree *)calloc(1, sizeof(BPTree));
}

void bptree_destroy(BPTree *tree)
{
    if (tree == NULL) {
        return;
    }

    bptree_destroy_node(tree->root);
    free(tree);
}

int bptree_insert(BPTree *tree, int key, void *value)
{
    BPTreeNode *leaf;

    if (tree == NULL) {
        return 0;
    }

    if (tree->root == NULL) {
        tree->root = bptree_create_node(1);
        if (tree->root == NULL) {
            return 0;
        }
    }

    leaf = bptree_find_leaf(tree, key);
    if (leaf == NULL) {
        leaf = tree->root;
    }

    if (bptree_search(tree, key) != NULL) {
        return 0;
    }

    bptree_insert_into_leaf(leaf, key, value);
    if (leaf->num_keys > BPTREE_MAX_KEYS) {
        return bptree_split_leaf(tree, leaf);
    }

    return 1;
}

void *bptree_search(BPTree *tree, int key)
{
    BPTreeNode *leaf;
    int index;

    leaf = bptree_find_leaf(tree, key);
    if (leaf == NULL) {
        return NULL;
    }

    for (index = 0; index < leaf->num_keys; index++) {
        if (leaf->keys[index] == key) {
            return leaf->values[index];
        }
    }

    return NULL;
}
