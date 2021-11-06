#ifndef EXPR_STATEMENT_H
#define EXPR_STATEMENT_H

typedef struct Expr Expr;

typedef struct ExprStatement {
    Expr* expr;
} ExprStatement;

ExprStatement* create_expr_statement(Expr* expr);

void free_expr_statement(ExprStatement* s);

#include "ast/expr.h"

#endif

