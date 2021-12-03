#ifndef EXPR_STATEMENT_H
#define EXPR_STATEMENT_H

#include "ast/expr.h"

typedef struct ExprStatement {
    Expr expr;
} ExprStatement;

ExprStatement* create_expr_statement(Expr expr);

void free_expr_statement(ExprStatement* s);

#endif

