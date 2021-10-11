#ifndef PRIMARY_EXPR_H
#define PRIMARY_EXPR_H

#include "token.h"

typedef struct Expr Expr;

typedef struct {
    bool is_bracket;
    union {
        struct {
            TokenType type;
            char* spelling;
        };
        Expr* bracket_expr;
    };
} PrimaryExpr;

PrimaryExpr* create_primary_expr(TokenType type, char* spelling);

PrimaryExpr* create_primary_expr_bracket(Expr* bracket_expr);

void free_primary_expr(PrimaryExpr* bracket_expr);

#include "ast/expr.h"

#endif