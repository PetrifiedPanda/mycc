#ifndef MUL_EXPR_H
#define MUL_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct CastExpr CastExpr;

typedef enum {
    MUL_EXPR_MUL,
    MUL_EXPR_DIV,
    MUL_EXPR_MOD,
} MulExprOp;

typedef struct {
    MulExprOp op;
    CastExpr* rhs;
} CastExprAndOp;

typedef struct MulExpr {
    CastExpr* lhs;
    size_t len;
    CastExprAndOp* mul_chain;
} MulExpr;

MulExpr* parse_mul_expr(ParserState* s);

MulExpr* parse_mul_expr_cast(ParserState* s, CastExpr* start);

void free_mul_expr(MulExpr* e);

#include "CastExpr.h"

#endif

