#ifndef SHIFT_EXPR_H
#define SHIFT_EXPR_H

#include <stddef.h>

#include "token_type.h"

typedef struct AddExpr AddExpr;

typedef struct {
    TokenType shift_op;
    AddExpr* add_expr;
} AddExprAndOp;

typedef struct ShiftExpr {
    size_t len;
    AddExprAndOp* shift_chain;
} ShiftExpr;

ShiftExpr* create_shift_expr(AddExprAndOp* shift_chain, size_t len);

void free_shift_expr(ShiftExpr* e);

#include "ast/add_expr.h"

#endif
