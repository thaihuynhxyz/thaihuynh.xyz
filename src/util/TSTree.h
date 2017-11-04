#ifndef BACKEND_TSTREE_H
#define BACKEND_TSTREE_H

#include <stddef.h>

#include "DArray.h"

typedef struct TSTree {
    char split_char;
    struct TSTree *low;
    struct TSTree *equal;
    struct TSTree *high;
    DArray *value;
} TSTree;

void *TSTree_search(TSTree *root, const char *key, size_t len);

void *TSTree_search_prefix(TSTree *root, const char *key, size_t len);

typedef void (*TSTree_traverse_cb)(void *value, void *data);

TSTree *TSTree_insert(TSTree *node, const char *key, size_t len, void *value);

void TSTree_traverse(TSTree *node, TSTree_traverse_cb cb, void *data);

void TSTree_destroy(TSTree *root);

#endif //BACKEND_TSTREE_H
