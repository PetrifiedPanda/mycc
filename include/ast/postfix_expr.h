#ifndef POSTFIX_EXPR_H
#define POSTFIX_EXPR_H

#include "token.h"

#include "ast/arg_expr_list.h"

typedef struct PrimaryExpr PrimaryExpr;
typedef struct Expr Expr;

typedef enum {
    POSTFIX_INDEX,
    POSTFIX_BRACKET,
    POSTFIX_ACCESS,
    POSTFIX_PTR_ACCESS,
    POSTFIX_INC_DEC
} PostfixSuffixType;

typedef struct {
    PostfixSuffixType type;
    union {
        Expr* index_expr;
        ArgExprList bracket_list;
        char* identifier;
        TokenType inc_dec;
    };
} PostfixSuffix;

typedef struct PostfixExpr {
    PrimaryExpr* primary;
    size_t len;
    PostfixSuffix* suffixes;
} PostfixExpr;

PostfixExpr* create_postfix_expr(PrimaryExpr* primary, PostfixSuffix* suffixes, size_t len);

void free_postfix_expr(PostfixExpr* p);

#include "ast/primary_expr.h"
#include "ast/expr.h"

#endif
