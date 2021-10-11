#ifndef MUL_EXPR_H
#define MUL_EXPR_H

#include "token.h"

typedef struct CastExpr CastExpr;

typedef struct {
    TokenType mul_op;
    CastExpr* cast_expr;
} CastExprAndOp;

typedef struct MulExpr {
    size_t len;
    CastExprAndOp* mul_chain;
} MulExpr;

MulExpr* create_mul_expr(CastExprAndOp* mul_chain, size_t len);

void free_mul_expr(MulExpr* e);

#include "ast/cast_expr.h"

#endif