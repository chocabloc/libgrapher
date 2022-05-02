#include <stdio.h>
#include "interface.h"
#include "expression.h"

int load_expr(const char* str) {
    expr_t* expr = expr_compile(str);
    if (expr == NULL)
        return -1;
    
    // TODO: book-keeping here

    return 0;
}
