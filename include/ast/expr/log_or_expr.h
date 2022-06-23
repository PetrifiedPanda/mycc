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

struct cast_expr;

struct log_or_expr* parse_log_or_expr_cast(struct parser_state* s,
                                           struct cast_expr* start);

void free_log_or_expr(struct log_or_expr* e);

struct ast_visitor;

bool visit_log_or_expr(struct ast_visitor* visitor, struct log_or_expr* e);

#include "ast/expr/log_and_expr.h"
#include "ast/expr/cast_expr.h"

#endif

