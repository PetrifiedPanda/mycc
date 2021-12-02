#ifndef PRIMARY_EXPR_H
#define PRIMARY_EXPR_H

#include "token_type.h"

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
        char* literal;
        Identifier* identifier;
        Expr* bracket_expr;
    };
} PrimaryExpr;

PrimaryExpr* create_primary_expr_constant(char* literal);

PrimaryExpr* create_primary_expr_string(char* literal);

PrimaryExpr* create_primary_expr_identifier(Identifier* identifier);

PrimaryExpr* create_primary_expr_bracket(Expr* bracket_expr);

void free_primary_expr(PrimaryExpr* bracket_expr);

#include "ast/expr.h"
#include "ast/identifier.h"

#endif

