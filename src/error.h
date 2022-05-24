#pragma once

#include "expression.h"
#include "parsing/tokens.h"
#include "parsing/ast.h"

void error_at_token(const char* str, expr_t* expr, token_t* tk);
void error_at_pos(const char* str, expr_t* expr, int pos);
