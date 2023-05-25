#ifndef ARG_EXPR_LST_H
#define ARG_EXPR_LST_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct AssignExpr AssignExpr;

typedef struct {
    size_t len;
    AssignExpr* assign_exprs;
} ArgExprList;

bool parse_arg_expr_list(ParserState* s, ArgExprList* res);

void free_arg_expr_list(ArgExprList* l);

#include "AssignExpr.h"

#endif

