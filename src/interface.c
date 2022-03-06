#include "interface.h"
#include "parser/parser.h"
#include <stdio.h>

int compile_expr(char* expr) {
    parser_make_ast(expr);
    return -1;
}
