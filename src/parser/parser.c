#include "parser.h"
#include "tokenizer.h"
#include "utils/vector.h"
#include <malloc.h>
#include <stdio.h>

ast_t* parser_make_ast(char* expr)
{
    // split expression into tokens first
    tokenlist_t* tokens = tk_tokenize(expr);
    if (tokens)
        tk_debug(tokens);
    else
        return NULL;

    // allocate the ast and initialize name table
    ast_t* ast = malloc(sizeof(ast_t));
    ast->name_table = tokens->name_table;

    // store index of parantheses in order of their opening

    return NULL;
}
