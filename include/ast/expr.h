#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

struct assign_expr;

struct expr {
    size_t len;
    struct assign_expr* assign_exprs;
};

void free_expr_children(struct expr* expr);

void free_expr(struct expr* expr);

#include "ast/assign_expr.h"

#endif

