#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

typedef struct AssignExpr AssignExpr;

typedef struct Expr {
    size_t len;
    AssignExpr* assign_exprs;
} Expr;

Expr* create_expr(AssignExpr* assign_exprs, size_t len);

void free_expr(Expr* expr);

#include "ast/assign_expr.h"

#endif
