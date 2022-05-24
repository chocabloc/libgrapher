#pragma once

#include <stdint.h>
#include "utils/hashmap.h"
#include "utils/vector.h"

// structure of an AST node
typedef struct ast_node {

    // type of node
    enum {
        NODE_TYPE_LITERAL,
        NODE_TYPE_FUNCTION,
        NODE_TYPE_OPERATOR,
        NODE_TYPE_VARIABLE
    } type;

    // node data
    union {
        float literal;
        int64_t name_id;
        uint8_t operator;
    } data;

    // children
    vec_struct(struct ast_node*) children;
} ast_node_t;
