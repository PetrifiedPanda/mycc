#ifndef PRIMARY_EXPR_H
#define PRIMARY_EXPR_H

#include "token_type.h"

#include "ast/constant.h"
#include "ast/string_literal.h"

typedef struct Expr Expr;
typedef struct Identifier Identifier;

typedef enum {
    PRIMARY_EXPR_IDENTIFIER,
    PRIMARY_EXPR_CONSTANT,
    PRIMARY_EXPR_STRING_LITERAL,
    PRIMARY_EXPR_BRACKET
} PrimaryExprType;

typedef struct PrimaryExpr {
    PrimaryExprType type;
    union {
        Constant constant;
        StringLiteral literal;
        Identifier* identifier;
        Expr* bracket_expr;
    };
} PrimaryExpr;

PrimaryExpr* create_primary_expr_constant(Constant constant);

PrimaryExpr* create_primary_expr_string(StringLiteral literal);

PrimaryExpr* create_primary_expr_identifier(Identifier* identifier);

PrimaryExpr* create_primary_expr_bracket(Expr* bracket_expr);

void free_primary_expr(PrimaryExpr* bracket_expr);

#include "ast/expr.h"
#include "ast/identifier.h"

#endif

