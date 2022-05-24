#pragma once

#include <stdint.h>
#include "ast.h"

// possible token types
typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_LITERAL,
    TOKEN_NAME,
    TOKEN_AST_FRAGMENT,
    TOKEN_OPERATOR,

    // separators
    TOKEN_OPENING_PAREN,
    TOKEN_CLOSING_PAREN,
    TOKEN_COMMA
} tokentype_t;

typedef struct token {
    tokentype_t type;
    union {
        float literal;
        uint8_t operator;
        int64_t name_id;
        ast_node_t* ast_frag;
    } data;

    struct token *next, *prev;
} token_t;

typedef struct {
    size_t num_tokens;
    token_t *first, *last;
} tokenlist_t;
