#ifndef CONST_EXPR_H
#define CONST_EXPR_H

#include "CondExpr.h"

#include "frontend/parser/ParserState.h"

typedef struct ConstExpr {
    CondExpr expr;
} ConstExpr;

ConstExpr* parse_const_expr(ParserState* s);

void ConstExpr_free(ConstExpr* e);

#endif

