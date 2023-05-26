#ifndef ADD_EXPR_H
#define ADD_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct MulExpr MulExpr;

typedef enum {
    ADD_EXPR_ADD,
    ADD_EXPR_SUB,
} AddExprOp;

typedef struct {
    AddExprOp op;
    MulExpr* rhs;
} MulExprAndOp;

typedef struct AddExpr {
    MulExpr* lhs;
    size_t len;
    MulExprAndOp* add_chain;
} AddExpr;

AddExpr* parse_add_expr(ParserState* s);

typedef struct CastExpr CastExpr;

AddExpr* parse_add_expr_cast(ParserState* s, CastExpr* start);

void AddExpr_free(AddExpr* e);

#include "MulExpr.h"
#include "CastExpr.h"

#endif

