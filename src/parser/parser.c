#include <stdio.h>
#include "parser.h"
#include "tokenizer.h"

ast_t* parser_make_ast(char* expr)
{
    tokenlist_t* tokens = tk_tokenize(expr);
    if (tokens)
        tk_debug(tokens);

    return NULL;
}
