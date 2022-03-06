#include "hashmap.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <math.h>

// combine hash and bucket index into a key
#define MAKE_KEY(hash, index) (((uint64_t)(index) << 32) | (hash))

static uint32_t get_hash(const char* str) {
    uint32_t h = 0, accum = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (i % 4 == 0) {
            h ^= accum * (i + 1);
            accum = 0;
        }
        accum <<= 8;
        accum += str[i];
    }
    h ^= accum;
    return h;
}

// create a hashmap
hashmap_t* hm_create(size_t n) {
    hashmap_t* hm = malloc(sizeof(hashmap_t) + n*sizeof(hm_bucket_t*));
    hm->num_buckets = n;
    hm->collisions = 0;
    for (size_t i = 0; i < n; i++) {
        size_t init_size = sizeof(hm_bucket_t) + 4 * sizeof(char*);
        hm_bucket_t* bkt = malloc(init_size);
        bkt->num_elems = 0;
        bkt->alloc_size = init_size;
        hm->buckets[i] = bkt;
    }
    return hm;
}

// return string, given its key
// NULL on error
const char* hm_get(hashmap_t* hm, int64_t key) {
    uint32_t hash = key & 0xffffffff, bkt_i = key >> 32;
    hm_bucket_t* bkt = hm->buckets[hash % hm->num_buckets];

    // no such string
    if (bkt_i >= bkt->num_elems)
        return NULL;

    return bkt->elems[bkt_i];
}

// check if string exists in map, and returns its key
// returns -1 if it doesn't exist
int64_t hm_find(hashmap_t* hm, const char* str) {
    uint32_t hash = get_hash(str);
    hm_bucket_t* bkt = hm->buckets[hash % hm->num_buckets];

    // find string in bucket
    for(size_t i = 0; i < bkt->num_elems; i++) {
        if (strcmp(str, bkt->elems[i]) == 0)
            return MAKE_KEY(hash, i);
    }

    // didn't find it
    return -1;
}

// adds string to hashmap, and returns its key
// return -1 on failure
int64_t hm_add(hashmap_t* hm, const char* str) {
    // does it already exist?
    if (hm_find(hm, str) != -1)
        return -1;

    uint32_t hash = get_hash(str);
    hm_bucket_t* bkt = hm->buckets[hash % hm->num_buckets];

    // increment element count and check for available space,
    // resizing if necessary
    bkt->num_elems++;
    size_t new_size = sizeof(hm_bucket_t) + (bkt->num_elems * sizeof(char*));
    if (bkt->alloc_size < new_size) {
        bkt->alloc_size = new_size*2;
        bkt = realloc(bkt, bkt->alloc_size);
        hm->buckets[hash % hm->num_buckets] = bkt;
    }

    // update item count and collision information
    hm->num_items++;
    hm->collisions += bkt->num_elems - 1;

    // add string to bucket and return its key
    bkt->elems[bkt->num_elems - 1] = str;
    return MAKE_KEY(hash, bkt->num_elems - 1);
}

// print some information about the hashmap
void hm_dbg(hashmap_t* hm) {
    double cov, mean = 0, mean_sq = 0,
           nb = (double)hm->num_buckets;
    for (size_t i = 0; i < hm->num_buckets; i++) {
        double ne = (double)hm->buckets[i]->num_elems;
        mean += ne / nb;
        mean_sq += (ne * ne) / nb;
    }
    cov = sqrt(mean_sq - (mean * mean)) / mean;
    printf("number of elements: %zu\nnumber of buckets: %zu\n", hm->num_items, hm->num_buckets);
    printf("distinct colliding pairs: %zu\ncollision score: %f\n", hm->collisions, cov);
}

// frees hashmap
// also frees data if asked to, ignoring const
void hm_free(hashmap_t* hm, bool free_str) {
    for (size_t i = 0; i < hm->num_buckets; i++) {
        if (free_str)
            for (size_t j = 0; j < hm->buckets[j]->num_elems; j++)
                free((void*)hm->buckets[i]->elems[j]);
        free(hm->buckets[i]);
    }
    free(hm);
}
