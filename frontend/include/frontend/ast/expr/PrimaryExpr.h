#ifndef PRIMARY_EXPR_H
#define PRIMARY_EXPR_H

#include "frontend/Token.h"

#include "Constant.h"
#include "StringConstant.h"

#include "frontend/parser/ParserState.h"

typedef struct Expr Expr;
typedef struct Identifier Identifier;
typedef struct GenericSel GenericSel;

typedef enum {
    PRIMARY_EXPR_IDENTIFIER,
    PRIMARY_EXPR_CONSTANT,
    PRIMARY_EXPR_STRING_LITERAL,
    PRIMARY_EXPR_BRACKET,
    PRIMARY_EXPR_GENERIC
} PrimaryExprKind;

typedef struct PrimaryExpr {
    PrimaryExprKind kind;
    union {
        Constant constant;
        StringConstant string;
        Identifier* identifier;
        struct {
            AstNodeInfo info;
            Expr* bracket_expr;
        };
        GenericSel* generic;
    };
} PrimaryExpr;

PrimaryExpr* parse_primary_expr(ParserState* s);

void PrimaryExpr_free(PrimaryExpr* bracket_expr);

#include "Expr.h"
#include "GenericSel.h"

#include "frontend/ast/Identifier.h"

#endif

