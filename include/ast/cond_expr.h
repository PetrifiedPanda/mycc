#ifndef COND_EXPR_H
#define COND_EXPR_H

#include <stddef.h>

struct log_or_expr;
struct expr;

struct log_or_and_expr {
    struct log_or_expr* log_or;
    struct expr* expr;
};

struct cond_expr {
    size_t len;
    struct log_or_and_expr* conditionals;
    struct log_or_expr* last_else;
};

struct cond_expr* create_cond_expr(struct log_or_and_expr* conditionals, size_t len, struct log_or_expr* last_else);

void free_cond_expr_children(struct cond_expr* e);
void free_cond_expr(struct cond_expr* e);

#include "ast/expr.h"
#include "ast/log_or_expr.h"

#endif

