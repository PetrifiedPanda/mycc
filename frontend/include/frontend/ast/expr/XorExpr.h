#ifndef XOR_EXPR_H
#define XOR_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct AndExpr AndExpr;

typedef struct XorExpr {
    size_t len;
    AndExpr* and_exprs;
} XorExpr;

bool parse_xor_expr_inplace(ParserState* s, XorExpr* res);

typedef struct CastExpr CastExpr;

XorExpr* parse_xor_expr_cast(ParserState* s, CastExpr* start);

void XorExpr_free_children(XorExpr* e);

#include "AndExpr.h"
#include "CastExpr.h"

#endif
