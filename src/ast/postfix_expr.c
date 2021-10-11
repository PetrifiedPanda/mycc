#include "ast/postfix_expr.h"

#include <assert.h>

PostfixExpr* create_postfix_expr_primary(PrimaryExpr* primary) {
    PostfixExpr* res = malloc(sizeof(PostfixExpr));
    if (res) {
        res->type = POSTFIX_PRIMARY;
        res->primary = primary;
        res->index_expr = NULL;
    }
    return res;
}

PostfixExpr* create_postfix_expr_index(PostfixExpr* postfix, Expr* index_expr) {
    PostfixExpr* res = malloc(sizeof(PostfixExpr));
    if (res) {
        res->type = POSTFIX_INDEX;
        res->postfix = postfix;
        index_expr = index_expr;
    }
    return res;
}

PostfixExpr* create_postfix_expr_fun_call(PostfixExpr* postfix, ArgExprLst* expr_list) {
    PostfixExpr* res = malloc(sizeof(PostfixExpr));
    if (res) {
        res->type = POSTFIX_FUN_CALL;
        res->postfix = postfix;
        expr_list = expr_list;
    }

    return res;
}

PostfixExpr* create_postfix_expr_access(PostfixType type, PostfixExpr* postfix, char* identifier) {
    assert(type == POSTFIX_ACCESS || type == POSTFIX_PTR_ACCESS);
    PostfixExpr* res = malloc(sizeof(PostfixExpr));
    if (res) {
        res->type = type;
        res->postfix = postfix;
        res->identifier = identifier;
    }

    return res;
}

PostfixExpr* create_postfix_expr_inc_dec(PostfixType type, PostfixExpr* postfix) {
    assert(type == POSTFIX_INC || type == POSTFIX_DEC);
    PostfixExpr* res = malloc(sizeof(PostfixExpr));
    if (res) {
        res->type = type;
        res->postfix = postfix;
        res->index_expr = NULL;
    }

    return res;
}

static void free_children(PostfixExpr* p) {
    switch (p->type) {
    case POSTFIX_PRIMARY:
        free_primary_expr(p->primary);
        return;
    case POSTFIX_INDEX:
        free_postfix_expr(p->postfix);
        free_expr(p->index_expr);
        return;
    case POSTFIX_FUN_CALL:
        free_postfix_expr(p->postfix);
        free_arg_expr_lst(p->expr_list);
        return;
    case POSTFIX_ACCESS:
    case POSTFIX_PTR_ACCESS:
        free_postfix_expr(p->postfix);
        free(p->identifier);
        return;
    case POSTFIX_INC:
        free_postfix_expr(p->postfix);
        return;
    }
}

void free_postfix_expr(PostfixExpr* p) {
    free_children(p);
    free(p);
}