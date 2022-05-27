#include <stdint.h>
#include <math.h>
#include "rt.h"
#include "utils/hashmap.h"

// operator definitions
// NULLed operators are implemented inline
operator_t rt_ops[UINT8_MAX + 1] = {
    ['='] = { .name = '=', .precedence = 100, .eval = NULL },
    ['+'] = { .name = '+', .precedence = 200, .eval = NULL },
    ['-'] = { .name = '-', .precedence = 200, .eval = NULL },
    ['*'] = { .name = '*', .precedence = 400, .eval = NULL },
    ['/'] = { .name = '/', .precedence = 400, .eval = NULL },
    ['^'] = { .name = '^', .precedence = 600, .eval = &powf }
};

// function definitions
// a -1 means variable number of arguments
static function_t rt_funcs[] = {
    { .name = "sin", .num_args = 1, .eval = NULL },
    { .name = "cos", .num_args = 1, .eval = NULL },
    { .name = "tan", .num_args = 1, .eval = NULL },
    { .name = "abs", .num_args = 1, .eval = NULL },
    { .name = "floor", .num_args = 1, .eval = NULL },
    { .name = "max", .num_args = -1, .eval = NULL },
    { .name = "min", .num_args = -1, .eval = NULL },
};
#define RT_NUM_FUNCS (sizeof(rt_funcs) / sizeof(rt_funcs[0]))

static hashmap_t* fn_map;

// get function information from name
function_t* rt_get_fn(const char* name)
{
    int64_t key = hm_find(fn_map, name);
    if (key != -1)
        return (function_t*)(hm_get(fn_map, key)->data);
    return NULL;
}

// initialize runtime
void rt_init() {
    // add all functions to hashmap
    // could probably be done at compile time
    // in a more powerful language
    fn_map = hm_create(HASHMAP_SIZE_DEFAULT);
    for (size_t i = 0; i < RT_NUM_FUNCS; i++)
        hm_add(fn_map, rt_funcs[i].name, (uint64_t)(&(rt_funcs[i])));
}
