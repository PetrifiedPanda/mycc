#ifndef OR_EXPR_H
#define OR_EXPR_H

#include <stddef.h>

struct xor_expr;

struct or_expr {
    size_t len;
    struct xor_expr* xor_exprs;
};

struct or_expr* create_or_expr(struct xor_expr* xor_exprs, size_t len);

void free_or_expr_children(struct or_expr* e);

void free_or_expr(struct or_expr* e);

#include "ast/xor_expr.h"

#endif

