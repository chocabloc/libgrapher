#include <stdio.h>
#include <string.h>
#include "error.h"

static void prettyprint(const char* str_msg, const char* str_expr, int from, int len) {
    printf("\033[0;31;1merror:\033[0m %s\n", str_msg);
    printf("\033[0;32;1mhere:\033[0m  ");

    // print expression with bad portion highlighted in red
    for (int i = 1; i < from; i++)
        putchar(str_expr[i]);
    printf("\033[0;31;1m");
    for (int i = from; i < from+len; i++)
        putchar(str_expr[i]);
    printf("\033[0m");
    for (int i = from + len; str_expr[i + 1] != '\0'; i++)
        putchar(str_expr[i]);
    printf("\n\033[0;31m");

    // draw squiggly line under error
    for (int i = 0; i < 6 + from; i++)
        printf(" ");
    printf("^");
    for (int i = 0; i < len - 1; i++)
        printf("~");
    printf("\033[0m\n");
}

void error_at_pos(const char* str, expr_t* expr, int pos) {
    prettyprint(str, expr->fn_str, pos, 1);
}

void error_at_token(const char* str, expr_t* expr, token_t* tk) {
    if (tk)
        prettyprint(str, expr->fn_str, tk->str_pos, tk->str_len);
    else
        prettyprint(str, expr->fn_str, strlen(str) - 2, 0);
}
