#ifndef ARG_EXPR_LST_H
#define ARG_EXPR_LST_H

#include <stddef.h>

typedef struct AssignExpr AssignExpr;

typedef struct ArgExprList {
    size_t len;
    AssignExpr* assign_exprs;
} ArgExprLst;

ArgExprLst* create_arg_expr_list(AssignExpr* assign_exprs, size_t len);

void free_arg_expr_list(ArgExprLst* list);

#include "ast/assign_expr.h"

#endif