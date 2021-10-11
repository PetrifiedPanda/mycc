#ifndef POSTFIX_EXPR_H
#define POSTFIX_EXPR_H

#include "ast/primary_expr.h"
#include "ast/arg_expr_lst.h"

typedef enum {
    POSTFIX_PRIMARY,
    POSTFIX_INDEX,
    POSTFIX_FUN_CALL,
    POSTFIX_ACCESS,
    POSTFIX_PTR_ACCESS,
    POSTFIX_INC,
    POSTFIX_DEC
} PostfixType;

typedef struct PostfixExpr {
    PostfixType type;
    union {
        struct PostfixExpr* postfix;
        PrimaryExpr* primary;
    };

    union {
        Expr* index_expr;
        ArgExprLst* expr_list;
        char* identifier;
    };
} PostfixExpr;

PostfixExpr* create_postfix_expr_primary(PrimaryExpr* primary);
PostfixExpr* create_postfix_expr_index(PostfixExpr* postfix, Expr* index_expr);
PostfixExpr* create_postfix_expr_fun_call(PostfixExpr* postfix, ArgExprLst* expr_list);
PostfixExpr* create_postfix_expr_access(PostfixType type, PostfixExpr* postfix, char* identifier);
PostfixExpr* create_postfix_expr_inc_dec(PostfixType type, PostfixExpr* postfix);

void free_postfix_expr(PostfixExpr* p);

#endif