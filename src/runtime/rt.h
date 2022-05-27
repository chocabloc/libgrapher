#pragma once

#include <stdint.h>
#include "utils/vector.h"
#include "expression.h"

typedef struct {
    uint8_t name;
    int precedence;
    float (*eval)(float, float);
} operator_t;

typedef struct {
    char* name;
    int num_args;
    float (*eval)(int num_args, float args[]);
} function_t;

typedef struct {
    char* name;
    float val;
    bool is_mut;
} variable_t;

extern operator_t rt_ops[];

void rt_init();
function_t* rt_get_fn(const char* name);
variable_t rt_get_var(const char* name);
int rt_resolve(expr_t* expr);
