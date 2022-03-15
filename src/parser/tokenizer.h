// TODO: complete the tokenizer

#pragma once
#include <complex.h>
#include <stdint.h>
#include "utils/hashmap.h"
#include "utils/vector.h"

// possible token type
typedef enum {
    TOKEN_TYPE_LITERAL,
    TOKEN_TYPE_NAME,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SEPARATOR
} tokentype_t;

typedef struct {
    tokentype_t type;
    union {
        complex double literal;
        char operator;
        char separator;
        int64_t name; // index into name table
    } data;
} token_t;

typedef struct {
    hashmap_t* name_table;
    vec_struct(token_t) tokens;
} tokenlist_t;

tokenlist_t* tk_tokenize(const char* expr);
void tk_debug(tokenlist_t* t);
