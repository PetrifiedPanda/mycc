#ifndef LOG_AND_EXPR_H
#define LOG_AND_EXPR_H

#include <stddef.h>

struct or_expr;

struct log_and_expr {
    size_t len;
    struct or_expr* or_exprs;
};

struct log_and_expr* create_log_and_expr(struct or_expr* or_exprs, size_t len);

void free_log_and_expr_children(struct log_and_expr* e);
void free_log_and_expr(struct log_and_expr* e);

#include "ast/or_expr.h"

#endif

