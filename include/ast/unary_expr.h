#ifndef UNARY_EXPR_H
#define UNARY_EXPR_H

#include "token.h"

typedef struct PostfixExpr PostfixExpr;
typedef struct CastExpr CastExpr;
typedef struct TypeName TypeName;

typedef enum {
    UNARY_POSTFIX,
    UNARY_INC_DEC,
    UNARY_UNARY_OP,
    UNARY_SIZEOF_UNARY,
    UNARY_SIZEOF_TYPENAME
} UnaryExprType;

typedef struct UnaryExpr {
    UnaryExprType type;
    union {
        PostfixExpr* postfix;
        struct {
            TokenType inc_dec;
            struct UnaryExpr* unary;
        };
        struct {
            TokenType unary_op;
            CastExpr* cast_expr;
        };
        struct UnaryExpr* sizeof_unary;
        TypeName* typename_unary;
    
    };
} UnaryExpr;

UnaryExpr* create_unary_expr_postfix(PostfixExpr* postfix);
UnaryExpr* create_unary_expr_inc_dec(TokenType inc_dec, UnaryExpr* unary);
UnaryExpr* create_unary_expr_unary_op(TokenType unary_op, CastExpr* cast_expr);
UnaryExpr* create_unary_expr_sizeof_unary(UnaryExpr* sizeof_unary);
UnaryExpr* create_unary_expr_sizeof_typename(TypeName* typename_unary);

void free_unary_expr_children(UnaryExpr* unary);
void free_unary_expr(UnaryExpr* unary);

#include "ast/postfix_expr.h"
#include "ast/cast_expr.h"
#include "ast/type_name.h"

#endif