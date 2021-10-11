#ifndef OR_EXPR_H
#define OR_EXPR_H

#include <stddef.h>

typedef struct XorExpr XorExpr;

typedef struct OrExpr {
    size_t len;
    XorExpr* xor_exprs;
} OrExpr;

OrExpr* create_or_expr(XorExpr* xor_exprs, size_t len);

void free_or_expr_children(OrExpr* e);

void free_or_expr(OrExpr* e);

#include "ast/xor_expr.h"

#endif