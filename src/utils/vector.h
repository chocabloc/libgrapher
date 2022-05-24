#pragma once

#include <malloc.h>
#include <stdint.h>

#define DEFAULT_SIZE 8

#define vec_struct(type)   \
    struct {               \
        size_t alloc_size; \
        size_t len;        \
        size_t iter;       \
        type* data;        \
    }

#define vec_new(type)                               \
    {                                               \
        .alloc_size = DEFAULT_SIZE * sizeof(type),  \
        .len = 0,                                   \
        .iter = 0,                                  \
        .data = malloc(DEFAULT_SIZE * sizeof(type)) \
    }

#define vec_push(vec, elem)                                              \
    do {                                                                 \
        (vec)->len++;                                                    \
        if ((vec)->alloc_size < (vec)->len * sizeof((vec)->data[0])) {   \
            (vec)->alloc_size = 2 * (vec)->len * sizeof((vec)->data[0]); \
            (vec)->data = realloc((vec)->data, (vec)->alloc_size);       \
        }                                                                \
        (vec)->data[(vec)->len - 1] = elem;                              \
    } while(0)

#define vec_destruct(vec)      \
    {                          \
        (vec)->alloc_size = 0; \
        (vec)->len = 0;        \
        free((vec)->data);     \
        (vec)->data = NULL;    \
    }

#define vec_at(vec, index) (vec)->data[index]

#define vec_iterate(vec, var)                                                               \
        if ((vec)->len > 0)                                                                 \
            for (typeof(*((vec)->data)) (var) = (vec)->data[0]; (vec)->iter < (vec)->len;   \
             (var) = (vec)->data[++((vec)->iter)])

#define vec_iterate_end(vec) (vec)->iter = 0
