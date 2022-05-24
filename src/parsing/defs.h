#pragma once

#include <stdint.h>
#include "utils/hashmap.h"
#include "utils/vector.h"

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

typedef struct ast_node_t ast_node_t;
typedef struct token {
    tokentype_t type;
    union {
        float literal;
        uint8_t operator;
        int64_t name_id;
        ast_node_t* ast_frag;
    } data;
    // size and position of token in expression string
    int str_pos, str_len;
    
    struct token *next, *prev;
} token_t;

typedef struct {
    size_t num_tokens;
    token_t *first, *last;
} tokenlist_t;

// structure of an AST node
struct ast_node_t {

    // type of node
    enum {
        NODE_TYPE_LITERAL,
        NODE_TYPE_FUNCTION,
        NODE_TYPE_OPERATOR,
        NODE_TYPE_VARIABLE
    } type;

    token_t* token;

    // children
    vec_struct(struct ast_node_t*) children;
};
