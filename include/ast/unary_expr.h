#ifndef UNARY_EXPR_H
#define UNARY_EXPR_H

#include "token_type.h"

typedef struct PostfixExpr PostfixExpr;
typedef struct CastExpr CastExpr;
typedef struct TypeName TypeName;

typedef enum {
    UNARY_POSTFIX,
    UNARY_UNARY_OP,
    UNARY_SIZEOF_TYPE
} UnaryExprType;

typedef struct UnaryExpr {
    size_t len;
    TokenType* operators_before; // only SIZEOF INC_OP or DEC_OP
    UnaryExprType type;
    union {
        PostfixExpr* postfix;
        struct {
            TokenType unary_op;
            CastExpr* cast_expr;
        };
        TypeName* type_name;
    };
} UnaryExpr;

UnaryExpr* create_unary_expr_postfix(TokenType* operators_before, size_t len, PostfixExpr* postfix);
UnaryExpr* create_unary_expr_unary_op(TokenType* operators_before, size_t len, TokenType unary_op, CastExpr* cast_expr);
UnaryExpr* create_unary_expr_sizeof_type(TokenType* operators_before, size_t len, TypeName* type_name);

void free_unary_expr_children(UnaryExpr* u);
void free_unary_expr(UnaryExpr* u);

#include "ast/postfix_expr.h"
#include "ast/cast_expr.h"
#include "ast/type_name.h"

#endif
