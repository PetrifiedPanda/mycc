#ifndef CONST_EXPR_H
#define CONST_EXPR_H

#include "ast/cond_expr.h"

typedef struct ConstExpr {
    CondExpr expr;
} ConstExpr;

ConstExpr* create_const_expr(CondExpr expr);

void free_const_expr(ConstExpr* e);

#endif

