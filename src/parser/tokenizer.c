#include <malloc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include "tokenizer.h"
#include "utils/hashmap.h"

// a reasonable size for a symbol table
#define DEFAULT_TABLE_SIZE 63

// get possible token type from starting character
static tokentype_t get_type(char c) {
    static const char* ops = "+-*/^!";
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return TOKEN_TYPE_NAME;
    else if (c >= '0' && c <= '9')
        return TOKEN_TYPE_LITERAL;
    else if (c == '(' || c == ')' || c == ',')
        return TOKEN_TYPE_SEPARATOR;
    for (size_t i = 0; ops[i] != '\0'; i++)
        if (ops[i] == c)
            return TOKEN_TYPE_OPERATOR;
    return -1;
}

tokenlist_t* tk_tokenize(const char* expr) {
    // allocate space for token list and its name table
    tokenlist_t* tlist = malloc(sizeof(tokenlist_t));
    *tlist = (tokenlist_t) { 
        .name_table = hm_create(DEFAULT_TABLE_SIZE),
        .tokens = vec_new(token_t)
    };

    while (*expr != '\0') {
        if (*expr == ' ') {
            expr++;
            continue;
        }

        token_t tk = { .type = get_type(*expr) };
        switch (tk.type) {
            case TOKEN_TYPE_OPERATOR: {
                tk.data.operator = *expr;
            } break;

            case TOKEN_TYPE_SEPARATOR: {
                tk.data.separator = *expr;
            } break;

            case TOKEN_TYPE_LITERAL: {
                complex double num = *(expr++) - '0';
                while (*expr >= '0' && *expr <= '9') {
                    num *= 10;
                    num += *(expr++) - '0';
                }
                if (*expr == 'i')
                    num *= 1.0i;
                else
                    expr--;
                tk.data.literal = num;
            } break;

            case TOKEN_TYPE_NAME: {
                // calculate length of name
                size_t len = 1;
                const char* start = expr;
                char* name;
                while (get_type(*(++expr)) == TOKEN_TYPE_NAME)
                    len++;

                // allocate and fill string
                name = malloc(len + 1);
                memcpy(name, start, len);
                name[len] = '\0';

                // add to name table, if it doesn't exist
                int64_t key = hm_find(tlist->name_table, name);
                if (key == -1)
                    tk.data.name = hm_add(tlist->name_table, name);
                else
                    tk.data.name = key;
                expr--;
            } break;

            // unexpected character
            default: {
                fprintf(stderr, "error: unexpected character '%c'\n", *expr);
                goto fail;
            } break;
        }
        vec_push(&(tlist->tokens), tk);
        expr++;
    }

    return tlist;

fail:
    hm_free(tlist->name_table, true);
    vec_destruct(&(tlist->tokens));
    free(tlist);
    return NULL;
}

// print token list in a human readable format
void tk_debug(tokenlist_t* tlist) {
    static char *types[] = {
        [TOKEN_TYPE_LITERAL] = "LITERAL",
        [TOKEN_TYPE_NAME] = "NAME",
        [TOKEN_TYPE_OPERATOR] = "OPERATOR",
        [TOKEN_TYPE_SEPARATOR] = "SEPARATOR"
    };

    size_t index = 0;
    vec_iterate(&(tlist)->tokens, t) {
        printf("[%zu] = { .type = %s, .value = ", index++, types[t.type]);
        switch (t.type) {
            case TOKEN_TYPE_NAME:
                printf("\"%s\"", hm_get(tlist->name_table, t.data.name));
                break;

            case TOKEN_TYPE_OPERATOR:
                printf("'%c'", t.data.operator);
                break;

            case TOKEN_TYPE_SEPARATOR:
                printf("'%c'", t.data.separator);
                break;
            
            case TOKEN_TYPE_LITERAL:
                printf("%.2f + %.2fi", creal(t.data.literal), cimag(t.data.literal));
                break;
        }
        printf(" }\n");
    } vec_iterate_end(&(tlist->tokens));
}
