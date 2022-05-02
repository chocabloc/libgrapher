#include <complex.h>
#include <string.h>
#include "expression.h"
#include "utils/hashmap.h"
#include "parsing/tokens.h"
#include "parsing/ast.h"
#include "parsing/parser.h"

// print expression data in human readable format
void expr_debug(expr_t* expr) {
    for (token_t* t = expr->tokens.first; t != NULL; t = t-> next) {
        token_dbg(expr, t);
    }
    printf("\n");
}

expr_t* expr_compile(const char* str) {
    // add parentheses before and after string for
    // easier processing
    char* pstr = malloc(strlen(str) + 3);
    sprintf(pstr, "(%s)", str);

    // allocate space for expression data
    expr_t* expr = malloc(sizeof(expr_t));
    *expr = (expr_t) {
        .fn_str = pstr,
        .name_table = hm_create(HASHMAP_SIZE_DEFAULT),
        .tokens = { 
            .num_tokens = 0,
            .first = NULL,
            .last = NULL
        },
        .ast_root = NULL
    };

    // parse the expression
    if (parser_tokenize(expr) == -1 )
        goto fail;
    
    if (parser_make_ast(expr) == -1)
        goto fail;

    // TODO: IR and machine code generation

fail:
    // TODO: deallocate here
    return NULL;
}
