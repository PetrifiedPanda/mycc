#ifndef COND_EXPR_H
#define COND_EXPR_H

#include <stdlib.h>

typedef struct LogOrExpr LogOrExpr;
typedef struct Expr Expr;

typedef struct {
    LogOrExpr* log_or;
    Expr* expr;
} LogOrAndExpr;

typedef struct CondExpr {
    size_t len;
    LogOrAndExpr* conditionals;
    LogOrExpr* last_else;
} CondExpr;

CondExpr* create_cond_expr(LogOrAndExpr* conditionals, size_t len, LogOrExpr* last_else);

void free_cond_expr(CondExpr* e);

#include "ast/expr.h"
#include "ast/log_or_expr.h"

#endif