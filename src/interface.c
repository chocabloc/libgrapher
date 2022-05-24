#include <stdio.h>
#include "interface.h"
#include "expression.h"
#include "runtime/rt.h"

void lg_init(void) {
    rt_init();
}

int lg_load(const char* str) {
    expr_t* expr = expr_compile(str);
    if (expr == NULL)
        return -1;
    
    // TODO: book-keeping here

    return 0;
}
