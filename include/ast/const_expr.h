#ifndef CONST_EXPR_H
#define CONST_EXPR_H

typedef struct CondExpr CondExpr;

// TODO: if possible make this not contain a pointer

typedef struct ConstExpr {
    CondExpr* expr;
} ConstExpr;

ConstExpr* create_const_expr(CondExpr* expr);

void free_const_expr(ConstExpr* e);

#include "ast/cond_expr.h"

#endif
