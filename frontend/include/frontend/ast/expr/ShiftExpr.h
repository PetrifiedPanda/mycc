#ifndef SHIFT_EXPR_H
#define SHIFT_EXPR_H

#include <stddef.h>

#include "frontend/Token.h"

#include "frontend/parser/ParserState.h"

typedef struct AddExpr AddExpr;

typedef enum {
    SHIFT_EXPR_LEFT,
    SHIFT_EXPR_RIGHT,
} ShiftExprOp;

typedef struct {
    ShiftExprOp op;
    AddExpr* rhs;
} AddExprAndOp;

typedef struct ShiftExpr {
    AddExpr* lhs;
    size_t len;
    AddExprAndOp* shift_chain;
} ShiftExpr;

ShiftExpr* parse_shift_expr(ParserState* s);

typedef struct CastExpr CastExpr;

ShiftExpr* parse_shift_expr_cast(ParserState* s, CastExpr* start);

void free_shift_expr(ShiftExpr* e);

#include "AddExpr.h"
#include "CastExpr.h"

#endif

