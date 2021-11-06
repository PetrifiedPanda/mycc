#ifndef MUL_EXPR_H
#define MUL_EXPR_H

#include <stddef.h>

#include "token_type.h"

typedef struct CastExpr CastExpr;

typedef struct {
    TokenType mul_op;
    CastExpr* rhs;
} CastExprAndOp;

typedef struct MulExpr {
    CastExpr* lhs;
    size_t len;
    CastExprAndOp* mul_chain;
} MulExpr;

MulExpr* create_mul_expr(CastExpr* lhs, CastExprAndOp* mul_chain, size_t len);

void free_mul_expr(MulExpr* e);

#include "ast/cast_expr.h"

#endif

