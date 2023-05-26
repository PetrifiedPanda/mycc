#ifndef OR_EXPR_H
#define OR_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct XorExpr XorExpr;

typedef struct OrExpr {
    size_t len;
    XorExpr* xor_exprs;
} OrExpr;

bool parse_or_expr_inplace(ParserState* s, OrExpr* res);

typedef struct CastExpr CastExpr;

OrExpr* parse_or_expr_cast(ParserState* s, CastExpr* start);

void OrExpr_free_children(OrExpr* e);

#include "XorExpr.h"
#include "CastExpr.h"

#endif
