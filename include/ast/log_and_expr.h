#ifndef LOG_AND_EXPR_H
#define LOG_AND_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct or_expr;

struct log_and_expr {
    size_t len;
    struct or_expr* or_exprs;
};

bool parse_log_and_expr_inplace(struct parser_state* s, struct log_and_expr* res);

struct unary_expr;

struct log_and_expr* parse_log_and_expr_unary(struct parser_state* s, struct unary_expr* start);

void free_log_and_expr_children(struct log_and_expr* e);
void free_log_and_expr(struct log_and_expr* e);

#include "ast/or_expr.h"
#include "ast/unary_expr.h"

#endif

