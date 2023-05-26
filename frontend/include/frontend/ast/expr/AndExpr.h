#ifndef AND_EXPR_H
#define AND_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct EqExpr EqExpr;

typedef struct AndExpr {
    size_t len;
    EqExpr* eq_exprs;
} AndExpr;

bool parse_and_expr_inplace(ParserState* s, AndExpr* res);

typedef struct CastExpr CastExpr;

AndExpr* parse_and_expr_cast(ParserState* s, CastExpr* start);

void AndExpr_free_children(AndExpr* e);

#include "EqExpr.h"
#include "CastExpr.h"

#endif

