#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct AssignExpr AssignExpr;

typedef struct Expr {
    size_t len;
    AssignExpr* assign_exprs;
} Expr;

bool parse_expr_inplace(ParserState* s, Expr* res);

Expr* parse_expr(ParserState* s);

void Expr_free_children(Expr* expr);

void Expr_free(Expr* expr);

#include "AssignExpr.h"

#endif

