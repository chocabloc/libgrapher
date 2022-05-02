#pragma once

#include "expression.h"

int parser_tokenize(expr_t* expr);
int parser_make_ast(expr_t* expr);
void parser_debug(expr_t* expr);
void token_dbg(expr_t* expr, token_t* t);
