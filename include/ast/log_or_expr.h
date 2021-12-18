#ifndef LOG_OR_EXPR_H
#define LOG_OR_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct log_and_expr;

struct log_or_expr {
    size_t len;
    struct log_and_expr* log_ands;
};

struct log_or_expr* parse_log_or_expr(struct parser_state* s);

void free_log_or_expr(struct log_or_expr* e);

#include "ast/log_and_expr.h"

#endif

