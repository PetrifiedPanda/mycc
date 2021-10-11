#ifndef EQ_EXPR_H
#define EQ_EXPR_H

#include "token.h"

typedef struct RelExpr RelExpr;

typedef struct {
    RelExpr* rel_expr;
    TokenType eq_neq_op;
} RelExprAndOp;

typedef struct EqExpr {
    size_t len;
    RelExprAndOp* eq_neq_chain;
    RelExpr* last_item;
} EqExpr;

void create_eq_expr_inplace(EqExpr* res, RelExprAndOp* eq_neq_chain, size_t len, RelExpr* last_item);

EqExpr* create_eq_expr(RelExprAndOp* eq_neq_chain, size_t len, RelExpr* last_item);

void free_eq_expr_children(EqExpr* e);

void free_eq_expr(EqExpr* e);

#include "ast/rel_expr.h"

#endif