#include "utils/vector.h"
#include "runtime/rt.h"
#include "expression.h"
#include "parsing/ast.h"

static int resolve_subtree(expr_t* expr, ast_node_t* t, int errors) {
    // only functions and variables need to be resolved
    if (t->type == NODE_TYPE_FUNCTION || t->type == NODE_TYPE_VARIABLE) {
        hm_elem_t* e = hm_get(expr->name_table, t->data.name_id);

        // unresolvable, don't bother with it
        if (e->data == UINT64_MAX)
            goto next;

        if (t->type == NODE_TYPE_FUNCTION)
            e->data = (uint64_t)rt_get_fn(e->str);
        else if (t->type == NODE_TYPE_VARIABLE) {
            // TODO
        }
        
        // name not found
        if (e->data == 0) {
            fprintf(stderr, "error: could not resolve name: %s\n", e->str);

            // mark it as unresolvable and increment error count
            e->data = UINT64_MAX;
            errors++;
        }
    }

next:
    // recursively operate on children
    vec_iterate(&(t->children), c) {
        resolve_subtree(expr, c, errors);
    } vec_iterate_end(&(t->children));
    return errors;
}


int rt_resolve(expr_t* expr) {
    int errors = resolve_subtree(expr, expr->ast_root, 0);
    return -1 * (errors > 0);
}
