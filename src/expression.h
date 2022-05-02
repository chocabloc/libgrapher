#pragma once

#include "utils/hashmap.h"
#include "parsing/tokens.h"
#include "parsing/ast.h"

typedef struct {
    const char* fn_str;
    hashmap_t* name_table;
    tokenlist_t tokens;
    ast_node_t* ast_root;
} expr_t;

expr_t* expr_compile(const char* str);
void expr_debug(expr_t* expr);
