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
    ['/'] = 500,
    ['*'] = 400,
    ['+'] = 300,
    ['-'] = 200
};

// insert operator into correct position in oplist
static void oplist_add(struct oplist** ops, token_t* t) {
    struct oplist* newop = malloc(sizeof(struct oplist));
    newop->op = t;

    // insert at beginning
    if (*ops == NULL || PRECEDENCE(t) >= PRECEDENCE((*ops)->op)) {
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

// convert an operable token to a node
static ast_node_t* operable_to_node(token_t* t) {
    if (t->type == TOKEN_AST_FRAGMENT)
        return t->data.ast_frag;
    else {
        ast_node_t* arg = malloc(sizeof(ast_node_t));
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

static void dbg_ast(expr_t* expr, ast_node_t* root, int lvl) {
    for (int i = 0; i < lvl; i++)
        printf(" ");
    
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

    vec_iterate(&(root->children), c) {
        dbg_ast(expr, c, lvl + 1);
    } vec_iterate_end(&(root->children));
}

int parser_make_ast(expr_t* expr)
{
    // store indices of parenthesis in order of their opening
    // TODO: complete ast generation
    vec_struct(token_t*) parens = vec_new(token_t*);
    for (token_t* tk = expr->tokens.last; tk != NULL; tk = tk->prev)
        if (tk->type == TOKEN_OPENING_PAREN)
            vec_push(&parens, tk);

    vec_iterate(&parens, p) {
        struct oplist* ops = NULL;
        token_t* cp = NULL;
        vec_struct(token_t*) args = vec_new(token_t*);
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
                    goto error;
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
                }
                expects = OPERABLE;
            }
        }

        // mismatched parens
        if (cp == NULL)
            goto error;

        // loop through operator list and convert each into an ast fragment
        // eating up the operator and its arguments in the process
        for (struct oplist* i = ops; i != NULL; i = i->next) {

            // check if it has valid arguments
            token_t* args[2] = { i->op->prev, i->op->next };
            for (int i = 0; i < 2; i++)
                if (args[i] == NULL || !IS_OPERABLE(args[i]))
                    goto error;

            // create node for the operator
            ast_node_t* op = malloc(sizeof(ast_node_t));
            *op = (ast_node_t) {
                .type = NODE_TYPE_OPERATOR,
                .data.operator= i->op->data.operator,
                .children = vec_new(ast_node_t*)
            };

            // create nodes for arguments
            for (int i = 0; i < 2; i++)
                vec_push(&(op->children), operable_to_node(args[i]));

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
                if (!IS_OPERABLE(c->next))
                    goto error;
                vec_push(&(fncall->children), operable_to_node(c->next));
            }

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

        expr_debug(expr);
    } vec_iterate_end(&parens);

    // the first token now contains the ast
    expr->ast_root = expr->tokens.first->data.ast_frag;
    dbg_ast(expr, expr->ast_root, 0);
    return 0;

error:
    // TODO: clean up allocated data
    printf("error during ast generation\n");
    return -1;
}
