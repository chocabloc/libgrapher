// TODO: complete the AST maker

#pragma once
#include <complex.h>
#include <stdint.h>
#include "utils/hashmap.h"

#define AST_NODE_MAX_CHILDREN 4

// structure of an AST node
typedef struct ast_node {

    // type of node
    enum {
        NODE_TYPE_LITERAL,
        NODE_TYPE_FUNCTION,
        NODE_TYPE_VARIABLE
    } type;

    // node data
    union {
        complex float literal;
        int64_t variable;
        int64_t function;
    } data;

    // children (maximum 4)
    struct ast_node* children[AST_NODE_MAX_CHILDREN];
} ast_node_t;

typedef struct {
    hashmap_t* name_table;
    ast_node_t* root;
} ast_t;
