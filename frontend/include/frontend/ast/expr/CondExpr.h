#ifndef COND_EXPR_H
#define COND_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct LogOrExpr LogOrExpr;
typedef struct Expr Expr;

typedef struct {
    LogOrExpr* log_or;
    Expr* expr;
} LogOrAndExpr;

typedef struct CondExpr {
    size_t len;
    LogOrAndExpr* conditionals;
    LogOrExpr* last_else;
} CondExpr;

bool parse_cond_expr_inplace(ParserState* s, CondExpr* res);

typedef struct CastExpr CastExpr;

CondExpr* parse_cond_expr_cast(ParserState* s, CastExpr* start);

void free_cond_expr_children(CondExpr* e);
void free_cond_expr(CondExpr* e);

#include "Expr.h"
#include "LogOrExpr.h"
#include "CastExpr.h"

#endif

