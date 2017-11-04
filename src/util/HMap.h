#ifndef BACKEND_HMAP_H
#define BACKEND_HMAP_H

#include <stdint.h>

#include "DArray.h"

typedef int (*HMap_compare)(char *a, char *b);

typedef uint32_t (*HMap_hash)(char *key);

typedef struct HMap {
    DArray *buckets;
    HMap_compare compare;
    HMap_hash hash;
} HMap;

struct hash_map_node {
    char *key;
    void *data;
    uint32_t hash;
};

typedef int (*HMap_traverse_cb)(struct hash_map_node *node);

HMap *HMap_create(HMap_compare compare, HMap_hash);

void HMap_destroy(HMap *map);

int HMap_set(HMap *map, char *key, void *data);

void *HMap_get(HMap *map, void *key);

int HMap_traverse(HMap *map, HMap_traverse_cb traverse_cb);

void *HMap_delete(HMap *map, void *key);

#endif //BACKEND_HMAP_H
