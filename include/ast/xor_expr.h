#ifndef XOR_EXPR_H
#define XOR_EXPR_H

#include <stddef.h>

typedef struct AndExpr AndExpr;

typedef struct XorExpr {
    size_t len;
    AndExpr* and_exprs;
} XorExpr;

XorExpr* create_xor_expr(AndExpr* and_exprs, size_t len);

void free_xor_expr_children(XorExpr* e);

void free_xor_expr(XorExpr* e);

#include "ast/and_expr.h"

#endif