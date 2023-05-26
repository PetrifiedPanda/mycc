#ifndef REL_EXPR_H
#define REL_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct ShiftExpr ShiftExpr;

typedef enum {
    REL_EXPR_LT,
    REL_EXPR_GT,
    REL_EXPR_LE,
    REL_EXPR_GE,
} RelExprOp;

typedef struct {
    RelExprOp op;
    ShiftExpr* rhs;
} ShiftExprAndOp;

typedef struct RelExpr {
    ShiftExpr* lhs;
    size_t len;
    ShiftExprAndOp* rel_chain;
} RelExpr;

RelExpr* parse_rel_expr(ParserState* s);

typedef struct CastExpr CastExpr;

RelExpr* parse_rel_expr_cast(ParserState* s, CastExpr* start);

void RelExpr_free_children(RelExpr* e);
void RelExpr_free(RelExpr* e);

#include "ShiftExpr.h"
#include "CastExpr.h"

#endif

