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

// a reasonable number of pre-allocated tokens
#define DEFAULT_NUM_TOKENS 63

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

// add token to tokenlist
static tokenlist_t* add_token(tokenlist_t* tl, token_t token) {
    tl->num_tokens++;

    // check new size and reallocate if needed
    size_t new_size = sizeof(tokenlist_t) + (tl->num_tokens) * sizeof(token_t);
    if (tl->alloc_size < new_size) {
        tl->alloc_size = 2 * new_size;
        tl = realloc(tl, tl->alloc_size);
    }

    // add token and return
    tl->tokens[tl->num_tokens - 1] = token;
    return tl;
}

tokenlist_t* tk_tokenize(const char* expr) {
    // allocate space for token list and its name table
    size_t alloc_size = sizeof(tokenlist_t) + DEFAULT_NUM_TOKENS * sizeof(token_t);
    tokenlist_t* tokens = malloc(alloc_size);
    tokens->alloc_size = alloc_size;
    tokens->name_table = hm_create(DEFAULT_TABLE_SIZE);

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
                int64_t key = hm_find(tokens->name_table, name);
                if (key == -1)
                    tk.data.name = hm_add(tokens->name_table, name);
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
        add_token(tokens, tk);
        expr++;
    }

    return tokens;

fail:
    hm_free(tokens->name_table, true);
    free(tokens);
    return NULL;
}

// print token list in a human readable format
void tk_debug(tokenlist_t* tokens) {
    static char *types[] = {
        [TOKEN_TYPE_LITERAL] = "LITERAL",
        [TOKEN_TYPE_NAME] = "NAME",
        [TOKEN_TYPE_OPERATOR] = "OPERATOR",
        [TOKEN_TYPE_SEPARATOR] = "SEPARATOR"
    };

    for (size_t i = 0; i < tokens->num_tokens; i++) {
        token_t t = tokens->tokens[i];
        printf("[%zu] = { .type = %s, .value = ", i, types[t.type]);
        switch (t.type) {
            case TOKEN_TYPE_NAME:
                printf("\"%s\"", hm_get(tokens->name_table, t.data.name));
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
    }
}
