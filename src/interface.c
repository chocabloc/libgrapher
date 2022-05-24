/*
    Interface provided to the outer world
    all functions in this file are exported
*/

#include <stdio.h>
#include "interface.h"
#include "expression.h"
#include "runtime/rt.h"

[[gnu::visibility("default")]] void lg_init(void) {
    rt_init();
}

[[gnu::visibility("default")]] int lg_load(const char* str) {
    expr_t* expr = expr_compile(str);
    if (expr == NULL)
        return -1;
    
    // TODO: book-keeping here

    return 0;
}
