#ifndef REL_EXPR_H
#define REL_EXPR_H

#include "token_type.h"

#include <stddef.h>

typedef struct ShiftExpr ShiftExpr;

typedef struct {
    TokenType rel_op;
    ShiftExpr* shift_expr;
} ShiftExprAndOp;

typedef struct RelExpr {
    size_t len;
    ShiftExprAndOp* rel_chain;
} RelExpr;

RelExpr* create_rel_expr(ShiftExprAndOp* rel_chain, size_t len);

void free_rel_expr_children(RelExpr* e);

void free_rel_expr(RelExpr* e);

#include "ast/shift_expr.h"

#endif
