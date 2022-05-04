#include "ast.h"
#include "tokens.h"
#include "../utils/vector.h"
#include "../expression.h"

#define IS_OPERABLE(t) ((t)->type < TOKEN_OPERATOR)
#define IS_OPERATOR(t) ((t)->type == TOKEN_OPERATOR)
#define PRECEDENCE(t) op_prec[(t)->data.operator]

// operator list sorted according to precedence
struct oplist {
    token_t* op;
    struct oplist* next;
};

// precedence order
static int op_prec[UINT8_MAX + 1] = {
    ['^'] = 600,    
    ['/'] = 400,
    ['*'] = 400,
    ['+'] = 200,
    ['-'] = 200
};

// insert operator into correct position in oplist
// (left-to-right associative)
static void oplist_add(struct oplist** ops, token_t* t) {
    struct oplist* newop = malloc(sizeof(struct oplist));
    newop->op = t;

    // insert at beginning
    if (*ops == NULL || PRECEDENCE(t) > PRECEDENCE((*ops)->op)) {
        newop->next = *ops;
        *ops = newop;
        return;
    }

    for (struct oplist* i = *ops; ; i = i->next) {
        // insert at end
        if (i->next == NULL) {
            i->next = newop;
            newop->next = NULL;
            return;
        }

        // insert in the middle
        if ((PRECEDENCE(i->op) >= PRECEDENCE(t)) && (PRECEDENCE(i->next->op) < PRECEDENCE(t))) {
            newop->next = i->next;
            i->next = newop;
            return;
        }
    }
}

// destruct an operator list
static void oplist_destruct(struct oplist** ops) {
    if (*ops == NULL)
        return;

    while ((*ops)->next) {
        struct oplist* tmp = (*ops)->next;
        free(*ops);
        *ops = tmp;
    }
    free(*ops);
    *ops = NULL;
}

// convert an operable token to a node
static ast_node_t* operable_to_node(token_t* t) {
    if (t->type == TOKEN_AST_FRAGMENT)
        return t->data.ast_frag;
    else {
        ast_node_t* arg = malloc(sizeof(ast_node_t));
        arg->children = (typeof(arg->children)) { 0 };
        
        if (t->type == TOKEN_NAME) {
            arg->type = NODE_TYPE_VARIABLE;
            arg->data.var_id = t->data.name_id;
        } else if (t->type == TOKEN_LITERAL) {
            arg->type = NODE_TYPE_LITERAL;
            arg->data.literal = t->data.literal;
        }
        return arg;
    }
}

// print the AST as a pretty tree
static void dbg_ast(expr_t* expr, ast_node_t* root, int lvl) {
    static vec_struct(bool) stems;
    if (stems.len == lvl)
        vec_push(&stems, true);
    else
        stems.data[lvl] = true;

    switch (root->type) {
        case NODE_TYPE_OPERATOR:
            printf("(%c)\n", root->data.operator);
            break;
        
        case NODE_TYPE_FUNCTION:
            printf("%s()\n", hm_get(expr->name_table, root->data.fun_id));
            break;

        case NODE_TYPE_VARIABLE:
            printf("%s\n", hm_get(expr->name_table, root->data.var_id));
            break;
        
        case NODE_TYPE_LITERAL:
            printf("%.2f + %.2fi\n", creal(root->data.literal), cimag(root->data.literal));
            break;
    }

    for (size_t i = 0; i < root->children.len; i++) {
        for (int j = 0; j < lvl; j++)
            printf(stems.data[j] ? " │" : "  ");

        if (i == root->children.len - 1) {
            stems.data[lvl] = false;
            printf(" └─");
        } else
            printf(" ├─");
        dbg_ast(expr, root->children.data[i], lvl + 1);
    }
}

int parser_make_ast(expr_t* expr)
{
    // macro for error handling
    #define ERROR(s)                           \
        {                                      \
            fprintf(stderr, "error: %s\n", s); \
            ret_val = -1;                      \
            goto end;                          \
        }

    // indicates success or error
    int ret_val = 0;

    // store indices of parenthesis in order of their opening
    // TODO: complete ast generation
    vec_struct(token_t*) parens = vec_new(token_t*);
    for (token_t* tk = expr->tokens.last; tk != NULL; tk = tk->prev)
        if (tk->type == TOKEN_OPENING_PAREN)
            vec_push(&parens, tk);

    struct oplist* ops = NULL;
    vec_struct(token_t*) args = { 0 };
    vec_iterate(&parens, p) {
        token_t* cp = NULL;
        vec_push(&args, p);

        // iterate through tokens inside the parentheses
        // and store operators according to precedence,
        // also making note of any arguments present
        // and convert operables into ast fragments
        enum { OPERATOR, OPERABLE } expects = OPERABLE;
        for (token_t* tk = p->next; tk != NULL; tk = tk->next) {
            if (expects == OPERABLE) {
                if (IS_OPERABLE(tk)) {
                    tk->data.ast_frag = operable_to_node(tk);
                    tk->type = TOKEN_AST_FRAGMENT;
                } else {
                    ERROR("expected operable value");
                }
                expects = OPERATOR;
            } else {
                if (tk->type == TOKEN_OPERATOR) {
                    oplist_add(&ops, tk);
                } else if (tk->type == TOKEN_CLOSING_PAREN) {
                    cp = tk;
                    break;
                } else if (tk->type == TOKEN_COMMA) {
                    vec_push(&args, tk);
                } else {
                    ERROR("expected operator, ')', or ','");
                }
                expects = OPERABLE;
            }
        }

        // mismatched parens
        if (cp == NULL)
            ERROR("mismatched parentheses detected");

        // loop through operator list and convert each into an ast fragment
        // eating up the operator and its arguments in the process
        for (struct oplist* i = ops; i != NULL; i = i->next) {
            token_t* args[2] = { i->op->prev, i->op->next };

            // create node for the operator
            ast_node_t* op = malloc(sizeof(ast_node_t));
            *op = (ast_node_t) {
                .type = NODE_TYPE_OPERATOR,
                .data.operator= i->op->data.operator,
                .children = vec_new(ast_node_t*)
            };

            // create nodes for arguments
            for (int j = 0; j < 2; j++)
                vec_push(&(op->children), operable_to_node(args[j]));

            // create token for the ast fragment
            token_t* fragtoken = malloc(sizeof(token_t));
            *fragtoken = (token_t) {
                .type = TOKEN_AST_FRAGMENT,
                .data.ast_frag = op,
                .next = args[1]->next,
                .prev = args[0]->prev

            };

            // remove the operator and its arguments and replace
            // it with the newly created ast fragment
            args[0]->prev->next = fragtoken;
            fragtoken->prev = args[0]->prev;
            args[1]->next->prev = fragtoken;
            fragtoken->next = args[1]->next;

            // deallocate operator and argument tokens
            free(i->op);
            free(args[0]);
            free(args[1]);
        }

        // token which will replace outer parens and its contents
        token_t* tkr = p->next;

        // if there's a name just before it, that means
        // it is a function call
        if (p->prev && p->prev->type == TOKEN_NAME) {

            // create the function call node and add
            // its arguments to it
            ast_node_t* fncall = malloc(sizeof(ast_node_t));
            *fncall = (ast_node_t) {
                .type = NODE_TYPE_FUNCTION,
                .data.fun_id = p->prev->data.name_id,
                .children = vec_new(ast_node_t*)
            };
            vec_iterate(&args, c) {
                vec_push(&(fncall->children), operable_to_node(c->next));
            } vec_iterate_end(&args);

            // create token for function call
            tkr = malloc(sizeof(token_t));
            tkr->type = TOKEN_AST_FRAGMENT;
            tkr->data.ast_frag = fncall;

            // we want to remove the name too,
            // not just the opening paren
            p = p->prev;
        }

        // add new token in place of parens and contents
        if (p->prev == NULL) {
            expr->tokens.first = tkr;
            tkr->prev = NULL;
        } else {
            p->prev->next = tkr;
            tkr->prev = p->prev;
        }
        if (cp->next == NULL) {
            expr->tokens.last = tkr;
            tkr->next = NULL;
        } else {
            cp->next->prev = tkr;
            tkr->next = cp->next;
        }

        // free parens and commas
        vec_iterate(&args, a) {
            free(a);
        } vec_iterate_end(&args);
        free(cp);

        // free operator and argument lists
        vec_destruct(&args);
        oplist_destruct(&ops);
    } vec_iterate_end(&parens);

    // the first token now contains the ast
    expr->ast_root = expr->tokens.first->data.ast_frag;
    dbg_ast(expr, expr->ast_root, 0);

end:
    // perform clean-up
    vec_destruct(&parens);
    if (ret_val == -1) {
        vec_destruct(&args);
        oplist_destruct(&ops);
        printf("aborting ast generation\n");
    }
    return ret_val;
}
