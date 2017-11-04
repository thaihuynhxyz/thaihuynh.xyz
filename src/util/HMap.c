#include <stdbool.h>

#include "HMap.h"

#define DEFAULT_NUMBER_OF_BUCKETS 100

static int default_compare(char *a, char *b) {
    return strcmp(a, b);
}

/**
 * Simple Bob Jenkins's hash algorithm taken from the
 * wikipedia description.
 */
static uint32_t default_hash(char *key) {
    const size_t len = strlen(key);
    uint32_t hash, i;

    for (hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}


HMap *HMap_create(HMap_compare compare, HMap_hash hash) {
    HMap *map = calloc(1, sizeof(HMap));
    check_mem(map);

    map->compare = compare ? compare : default_compare;
    map->hash = hash ? hash : default_hash;
    map->buckets = DArray_create(sizeof(DArray *), DEFAULT_NUMBER_OF_BUCKETS);
    map->buckets->end = map->buckets->max; // fake out expanding it
    check_mem(map->buckets);

    return map;

    error:
    if (map) HMap_destroy(map);

    return NULL;
}

void HMap_destroy(HMap *map) {
    int i, j;

    if (map) {
        if (map->buckets) {
            for (i = 0; i < DArray_count(map->buckets); i++) {
                DArray *bucket = DArray_get(map->buckets, i);
                if (bucket) {
                    for (j = 0; j < DArray_count(bucket); j++)
                        free(DArray_get(bucket, j));
                    DArray_destroy(bucket);
                }
            }
            DArray_destroy(map->buckets);
        }

        free(map);
    }
}

static inline struct hash_map_node *HMap_node_create(uint32_t hash, char *key, void *data) {
    struct hash_map_node *node = calloc(1, sizeof(struct hash_map_node));
    check_mem(node);

    node->key = key;
    node->data = data;
    node->hash = hash;

    return node;

    error:
    return NULL;
}

static inline DArray *HMap_find_bucket(HMap *map, char *key, bool is_create, uint32_t *hash_out) {
    uint32_t hash = map->hash(key);
    int bucket_n = hash % DEFAULT_NUMBER_OF_BUCKETS;
    check(bucket_n >= 0, "Invalid bucket found: %d", bucket_n);
    *hash_out = hash; // store it for the return so the caller can use it

    DArray *bucket = DArray_get(map->buckets, bucket_n);

    if (!bucket && is_create) {
        // new bucket, set it up
        bucket = DArray_create(sizeof(void *), DEFAULT_NUMBER_OF_BUCKETS);
        check_mem(bucket);
        DArray_set(map->buckets, bucket_n, bucket);
    }

    return bucket;

    error:
    return NULL;
}

int HMap_set(HMap *map, char *key, void *data) {
    uint32_t hash;
    DArray *bucket = HMap_find_bucket(map, key, true, &hash);
    check(bucket, "Error can't create bucket.");

    struct hash_map_node *node = HMap_node_create(hash, key, data);
    check_mem(node);

    DArray_push(bucket, node);

    return 0;

    error:
    return -1;
}

static inline int HMap_get_node(HMap *map, uint32_t hash, DArray *bucket, void *key) {
    int i = 0;

    for (i = 0; i < DArray_end(bucket); i++) {
        struct hash_map_node *node = DArray_get(bucket, i);
        if (node->hash == hash && !map->compare(node->key, key)) return i;
    }

    return -1;
}

void *HMap_get(HMap *map, void *key) {
    uint32_t hash;
    DArray *bucket = HMap_find_bucket(map, key, false, &hash);
    if (!bucket) return NULL;

    const int i = HMap_get_node(map, hash, bucket, key);
    if (i == -1) return NULL;

    struct hash_map_node *node = DArray_get(bucket, i);
    check(node != NULL, "Failed to get node from bucket when it should exist.");

    return node->data;

    error: // fallthrough
    return NULL;
}


int HMap_traverse(HMap *map, HMap_traverse_cb traverse_cb) {
    int i, j, rc;
    DArray *bucket;

    for (i = 0; i < DArray_count(map->buckets); i++) {
        bucket = DArray_get(map->buckets, i);
        if (bucket)
            for (j = 0; j < DArray_count(bucket); j++) {
                struct hash_map_node *node = DArray_get(bucket, j);
                rc = traverse_cb(node);
                if (rc != 0) return rc;
            }
    }

    return 0;
}

void *HMap_delete(HMap *map, void *key) {
    uint32_t hash = 0;
    DArray *bucket = HMap_find_bucket(map, key, false, &hash);
    if (!bucket) return NULL;

    int i = HMap_get_node(map, hash, bucket, key);
    if (i == -1) return NULL;

    struct hash_map_node *node = DArray_get(bucket, i);
    void *data = node->data;
    free(node);

    struct hash_map_node *ending = DArray_pop(bucket);

    // alright looks like it's not the last one, swap it
    if (ending != node) DArray_set(bucket, i, ending);

    return data;
}
