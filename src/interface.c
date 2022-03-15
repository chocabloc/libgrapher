#include "interface.h"
#include "parser/parser.h"
#include <stdio.h>

int compile_expr(char* expr) {
    if (!parser_make_ast(expr))
        return -1;
    return -1;
}
