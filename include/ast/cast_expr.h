#ifndef CAST_EXPR_H
#define CAST_EXPR_H

#include <stdlib.h>

typedef struct UnaryExpr UnaryExpr;
typedef struct TypeName TypeName;

typedef struct CastExpr {
    size_t len;
    TypeName* type_names;
    UnaryExpr* rhs;
} CastExpr;

CastExpr* create_cast_expr(TypeName* type_names, size_t len, UnaryExpr* rhs);

void free_cast_expr(CastExpr* e);

#include "ast/unary_expr.h"
#include "ast/type_name.h"

#endif

