#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>
#include "tokens.h"
#include "parser.h"
#include "expression.h"
#include "utils/hashmap.h"

static tokentype_t ttypes[UINT8_MAX + 1] = {
    ['+'] = TOKEN_OPERATOR,
    ['-'] = TOKEN_OPERATOR,
    ['*'] = TOKEN_OPERATOR,
    ['/'] = TOKEN_OPERATOR,
    ['^'] = TOKEN_OPERATOR,
    ['('] = TOKEN_OPENING_PAREN,
    [')'] = TOKEN_CLOSING_PAREN,
    [','] = TOKEN_COMMA
};

// get possible token type from starting character
static tokentype_t get_type(uint8_t c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return TOKEN_NAME;
    else if (c >= '0' && c <= '9')
        return TOKEN_LITERAL;

    return ttypes[c];
}

static void tlist_add(tokenlist_t* l, token_t* t) {
    t->prev = l->last;
    t->next = NULL;
    if (l->last == NULL) {
        l->last = t;
        l->first = t;
        return;
    }
    l->last->next = t;
    l->last = t;
}

int parser_tokenize(expr_t* expr) {
    char* str = (char*) expr->fn_str;

    while (*str != '\0') {
        if (*str == ' ') {
            str++;
            continue;
        }

        token_t* tk = malloc(sizeof(token_t));
        tk->type = get_type(*str);

        switch (tk->type) {
            case TOKEN_OPERATOR: {
                tk->data.operator = *str;
            } break;

            case TOKEN_LITERAL: {
                complex double num = *(str++) - '0';
                int dec_places = 0;

                while (*str >= '0' && *str <= '9') {
                    num *= 10;
                    num += *(str++) - '0';
                }
                if (*str == '.') {
                    str++;
                    while (*str >= '0' && *str <= '9') {
                        dec_places++;
                        num += (*(str++) - '0') / pow(10, dec_places);
                    }
                }
                if (*str == 'i')
                    num *= 1.0i;
                else
                    str--;
                tk->data.literal = num;
            } break;

            case TOKEN_NAME: {
                // calculate length of name
                size_t len = 1;
                const char* start = str;
                char* name;
                while (get_type(*(++str)) == TOKEN_NAME)
                    len++;

                // allocate and fill string
                name = malloc(len + 1);
                memcpy(name, start, len);
                name[len] = '\0';

                // add to name table, if it doesn't exist
                int64_t key = hm_find(expr->name_table, name);
                if (key == -1)
                    tk->data.name_id = hm_add(expr->name_table, name);
                else
                    tk->data.name_id = key;
                str--;
            } break;

            case TOKEN_UNKNOWN: {
                fprintf(stderr, "error: unexpected character '%c'\n", *str);
                free(tk);
                return -1;
            } break;
        }
        tlist_add(&(expr->tokens), tk);
        str++;
    }
    return 0;
}

void token_dbg(expr_t* expr, token_t* t) {
    static char* types[] = {
        [TOKEN_OPENING_PAREN] = "(",
        [TOKEN_CLOSING_PAREN] = ")",
        [TOKEN_COMMA] = ", ",
    };

    switch (t->type) {
        case TOKEN_NAME:
            printf("%s", hm_get(expr->name_table, t->data.name_id));
            break;

        case TOKEN_OPERATOR:
            printf(" %c ", t->data.operator);
            break;
            
        case TOKEN_LITERAL:
            printf("%.2f + %.2fi", creal(t->data.literal), cimag(t->data.literal));
            break;
            
        case TOKEN_AST_FRAGMENT:
            printf("(...)");
            break;

        default:
            printf("%s", types[t->type]);
    }
}
