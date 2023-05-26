#ifndef EQ_EXPR_H
#define EQ_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct RelExpr RelExpr;

typedef enum {
    EQ_EXPR_EQ,
    EQ_EXPR_NE,
} EqExprOp;

typedef struct {
    EqExprOp op;
    RelExpr* rhs;
} RelExprAndOp;

typedef struct EqExpr {
    RelExpr* lhs;
    size_t len;
    RelExprAndOp* eq_chain;
} EqExpr;

bool parse_eq_expr_inplace(ParserState* s, EqExpr* res);

typedef struct CastExpr CastExpr;

EqExpr* parse_eq_expr_cast(ParserState* s, CastExpr* start);

void EqExpr_free_children(EqExpr* e);

#include "RelExpr.h"
#include "CastExpr.h"

#endif

