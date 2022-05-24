#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HASHMAP_SIZE_DEFAULT 63

// string and the data associated with it
typedef struct {
    const char* str;
    uint64_t data;
} hm_elem_t;

typedef struct hm_bucket {
    size_t num_elems, alloc_size;
    hm_elem_t elems[];
} hm_bucket_t;

// maps string -> integer
typedef struct {
    size_t num_buckets;
    size_t num_items;
    size_t collisions;
    hm_bucket_t* buckets[];
} hashmap_t;

hashmap_t* hm_create(size_t n);
void hm_free(hashmap_t* hm);
int64_t hm_find(hashmap_t* hm, const char* str);
int64_t hm_add(hashmap_t* hm, const char* str, uint64_t data);
hm_elem_t* hm_get(hashmap_t* hm, int64_t val);
void hm_dbg(hashmap_t* hm);
