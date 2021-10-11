#ifndef ADDITIVE_EXPR_H
#define ADDITIVE_EXPR_H

#include "token.h"

typedef struct MulExpr MulExpr;

typedef struct {
    TokenType add_op;
    MulExpr* rhs;
} MulExprAndOp;

typedef struct AddExpr {
    MulExpr* lhs;
    size_t len;
    MulExprAndOp* add_chain;
} AddExpr;

AddExpr* create_add_expr(MulExpr* lhs, size_t len, MulExprAndOp* add_chain);

void free_add_expr(AddExpr* e);

#include "ast/mul_expr.h"

#endif