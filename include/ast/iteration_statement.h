#ifndef ITERATION_STATEMENT_H
#define ITERATION_STATEMENT_H

#include "token_type.h"

typedef struct Statement Statement;
typedef struct ExprStatement ExprStatement;
typedef struct Expr Expr;

typedef struct IterationStatement {
    TokenType type;
    Statement* loop_body;
    union {
        Expr* while_cond;
        struct {
            ExprStatement* init_expr;
            ExprStatement* for_cond;
            Expr* incr_expr;
        };
    };
} IterationStatement;

IterationStatement* create_while_loop(Expr* while_cond, Statement* loop_body);
IterationStatement* create_do_loop(Expr* while_cond, Statement* loop_body);
IterationStatement* create_for_loop(ExprStatement* init_expr, ExprStatement* for_cond, Expr* incr_expr, Statement* loop_body);

void free_iteration_statement(IterationStatement* s);

#include "ast/statement.h"
#include "ast/expr_statement.h"
#include "ast/expr.h"

#endif
