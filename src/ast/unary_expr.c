#include "ast/unary_expr.h"

#include <stdlib.h>
#include <assert.h>

UnaryExpr* create_unary_expr_postfix(PostfixExpr* postfix) {
    UnaryExpr* res = malloc(sizeof(UnaryExpr));
    if (res) {
        res->type = UNARY_POSTFIX;
        res->postfix = postfix;
    }
    return res;
}

UnaryExpr* create_unary_expr_inc_dec(TokenType inc_dec, UnaryExpr* unary) {
    assert(inc_dec == INC_OP || inc_dec == DEC_OP);
    UnaryExpr* res = malloc(sizeof(UnaryExpr));
    if (res) {
        res->type = UNARY_INC_DEC;
        res->inc_dec = inc_dec;
        res->unary = unary;
    }
    return res;
}

UnaryExpr* create_unary_expr_unary_op(TokenType unary_op, CastExpr* cast_expr) {
    assert(false); // TODO: assert 
    UnaryExpr* res = malloc(sizeof(UnaryExpr));
    if (res) {
        res->type = UNARY_UNARY_OP;
        res->unary_op = unary_op;
        res->cast_expr = cast_expr;
    }
    return res;
}
UnaryExpr* create_unary_expr_sizeof_unary(UnaryExpr* sizeof_unary) {
    UnaryExpr* res = malloc(sizeof(UnaryExpr));
    if (res) {
        res->type = UNARY_SIZEOF_UNARY;
        res->sizeof_unary = sizeof_unary;
    }
    return res;
}
UnaryExpr* create_unary_expr_sizeof_typename(TypeName* typename_unary) {
    UnaryExpr* res = malloc(sizeof(UnaryExpr));
    if (res) {
        res->type = UNARY_SIZEOF_TYPENAME;
        res->typename_unary = typename_unary;
    }
    return res;
}

void free_unary_expr_children(UnaryExpr* unary) {
    switch (unary->type) {
    case UNARY_POSTFIX:
        free_postfix_expr(unary->postfix);
        return;
    case UNARY_INC_DEC:
        free_unary_expr(unary->unary);
        return;
    case UNARY_UNARY_OP:
        free_cast_expr(unary->cast_expr);
        return;
    case UNARY_SIZEOF_UNARY:
        free_unary_expr(unary->sizeof_unary);
        return;
    case UNARY_SIZEOF_TYPENAME:
        free_type_name(unary->typename_unary);
        return;
    }
}

void free_unary_expr(UnaryExpr* unary) {
    free_unary_expr_children(unary);
    free(unary);
}