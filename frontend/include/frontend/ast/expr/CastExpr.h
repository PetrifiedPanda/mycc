#ifndef CAST_EXPR_H
#define CAST_EXPR_H

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"

typedef struct UnaryExpr UnaryExpr;
typedef struct TypeName TypeName;

typedef struct CastExpr {
    AstNodeInfo info;
    size_t len;
    TypeName* type_names;
    UnaryExpr* rhs;
} CastExpr;

CastExpr* parse_cast_expr(ParserState* s);
CastExpr* parse_cast_expr_type_name(ParserState* s, TypeName* type_name, SourceLoc start_bracket_loc);

CastExpr* CastExpr_create_unary(UnaryExpr* start);

void CastExpr_free(CastExpr* e);

#include "UnaryExpr.h"
#include "frontend/ast/TypeName.h"

#endif

