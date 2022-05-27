#include "ast.h"
#include "tokens.h"
#include "../utils/vector.h"
#include "../expression.h"
#include "../runtime/rt.h"
#include "error.h"

#define IS_OPERABLE(t) ((t)->type < TOKEN_OPERATOR)
#define IS_OPERATOR(t) ((t)->type == TOKEN_OPERATOR)
#define PRECEDENCE(t) rt_ops[(t)->data.operator].precedence

// operator list sorted according to precedence
struct oplist {
    token_t* op;
    struct oplist* next;
};

// insert operator into correct position in oplist
// (left-to-right associative)
static void oplist_add(struct oplist** ops, token_t* t) {
    struct oplist* newop = tmalloc(sizeof(struct oplist));
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
        tfree(*ops);
        *ops = tmp;
    }
    tfree(*ops);
    *ops = NULL;
}

// convert an operable token to a node
static ast_node_t* operable_to_node(token_t* t) {
    if (t->type == TOKEN_AST_FRAGMENT) {
        ast_node_t* ast_frag = t->data.ast_frag;
        tfree(t);
        return ast_frag;
    } else {
        ast_node_t* arg = tmalloc(sizeof(ast_node_t));
        arg->children = (typeof(arg->children)) { 0 };

        if (t->type == TOKEN_NAME)
            arg->type = NODE_TYPE_VARIABLE;
        else if (t->type == TOKEN_LITERAL)
            arg->type = NODE_TYPE_LITERAL;
        arg->token = *t;
        return arg;
    }
}

// print the AST as a pretty tree
static void dbg_ast(expr_t* expr, ast_node_t* root, size_t lvl) {
    static vec_struct(bool) stems;
    if (stems.len == lvl)
        vec_push(&stems, true);
    else
        stems.data[lvl] = true;

    token_t tk = root->token;
    switch (root->type) {
        case NODE_TYPE_OPERATOR:
            printf("(%c)\n", tk.data.operator);
            break;
        
        case NODE_TYPE_FUNCTION:
        case NODE_TYPE_VARIABLE:
            printf("%s\n", hm_get(expr->name_table, tk.data.name_id)->str);
            break;
        
        case NODE_TYPE_LITERAL:
            printf("%.2f\n", tk.data.literal);
            break;
    }

    for (size_t i = 0; i < root->children.len; i++) {
        for (size_t j = 0; j < lvl; j++)
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
    // indicates success or error
    int ret_val = 0;

    // store indices of parenthesis in order of their opening
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
                // convert names and literals to ast fragments
                if (tk->type == TOKEN_NAME || tk->type == TOKEN_LITERAL) {
                    tk->data.ast_frag = operable_to_node(tk);
                    tk->type = TOKEN_AST_FRAGMENT;
                } else if (!IS_OPERABLE(tk)) {
                    error_at_token("expected operable value", expr, tk);
                    ret_val = -1;
                    goto end;
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
                    error_at_token("expected operator, ')', or ','", expr, tk);
                    ret_val = -1;
                    goto end;
                }
                expects = OPERABLE;
            }
        }

        // mismatched parens
        if (cp == NULL) {
            error_at_token("expected closing bracket", expr, NULL);
            ret_val = -1;
            goto end;
        }

        // loop through operator list and convert each into an ast fragment
        // eating up the operator and its arguments in the process
        for (struct oplist* i = ops; i != NULL; i = i->next) {
            token_t* args[2] = { i->op->prev, i->op->next };

            // check if its an assignment and
            // whether its LHS is a variable
            if (i->op->data.operator == '=' && args[0]->data.ast_frag->token.type != TOKEN_NAME) {
                    error_at_token("LHS of '=' must be a variable", expr, i->op);
                    ret_val = -1;
                    goto end;
            }

            // create node for the operator
            ast_node_t* op = tmalloc(sizeof(ast_node_t));
            *op = (ast_node_t) {
                .type = NODE_TYPE_OPERATOR,
                .children = vec_new(ast_node_t*)
            };
            op->token = *(i->op);

            // create nodes for arguments
            for (int j = 0; j < 2; j++)
                vec_push(&(op->children), operable_to_node(args[j]));

            // create token for the ast fragment
            token_t* fragtoken = tmalloc(sizeof(token_t));
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
        }

        // token which will replace outer parens and its contents
        token_t* tkr = p->next;

        // if there's a name just before it, that means
        // it is a function call
        if (p->prev && p->prev->type == TOKEN_NAME) {

            // create the function call node
            ast_node_t* fncall = tmalloc(sizeof(ast_node_t));
            *fncall = (ast_node_t) {
                .type = NODE_TYPE_FUNCTION,
                .children = vec_new(ast_node_t*)
            };
            fncall->token = *(p->prev);

            // add its arguments to it
            vec_iterate(&args, c) {
                vec_push(&(fncall->children), operable_to_node(c->next));
            } vec_iterate_end(&args);

            // create token for function call
            tkr = tmalloc(sizeof(token_t));
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
            tfree(a);
        } vec_iterate_end(&args);
        tfree(cp);

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
    }
    return ret_val;
}
