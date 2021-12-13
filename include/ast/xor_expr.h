#ifndef XOR_EXPR_H
#define XOR_EXPR_H

#include <stddef.h>

struct and_expr;

struct xor_expr {
    size_t len;
    struct and_expr* and_exprs;
};

struct xor_expr* create_xor_expr(struct and_expr* and_exprs, size_t len);

void free_xor_expr_children(struct xor_expr* e);

void free_xor_expr(struct xor_expr* e);

#include "ast/and_expr.h"

#endif

