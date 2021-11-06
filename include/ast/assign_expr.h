#ifndef ASSIGN_EXPR_H
#define ASSIGN_EXPR_H

#include <stddef.h>

#include "token_type.h"

typedef struct UnaryExpr UnaryExpr;
typedef struct CondExpr CondExpr;

typedef struct {
    UnaryExpr* unary;
    TokenType assign_op;
} UnaryAndOp;

typedef struct AssignExpr {
    size_t len;
    UnaryAndOp* assign_chain;
    CondExpr* value;
} AssignExpr;

void create_assign_expr_inplace(AssignExpr* res, UnaryAndOp* assign_chain, size_t len, CondExpr* value);
AssignExpr* create_assign_expr(UnaryAndOp* assign_chain, size_t len, CondExpr* value);

void free_assign_expr_children(AssignExpr* e);
void free_assign_expr(AssignExpr* e);

#include "ast/cond_expr.h"
#include "ast/unary_expr.h"

#endif

