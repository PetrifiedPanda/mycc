#ifndef EQ_EXPR_H
#define EQ_EXPR_H

#include <stddef.h>

#include "token_type.h"

typedef struct RelExpr RelExpr;

typedef struct { 
    TokenType eq_op;
    RelExpr* rhs;
} RelExprAndOp;

typedef struct EqExpr {
    RelExpr* lhs;
    size_t len;
    RelExprAndOp* eq_chain; 
} EqExpr;

void init_eq_expr(EqExpr* res, RelExpr* lhs, RelExprAndOp* eq_chain, size_t len);

EqExpr* create_eq_expr(RelExpr* lhs, RelExprAndOp* eq_chain, size_t len);

void free_eq_expr_children(EqExpr* e);

void free_eq_expr(EqExpr* e);

#include "ast/rel_expr.h"

#endif

