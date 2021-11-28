#include "ast/unary_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static inline void assign_operators_before(UnaryExpr* res, TokenType* operators_before, size_t len) {
    assert(res);
    if (len > 0) {
        assert(operators_before);
    } else {
        assert(operators_before == NULL);
    }
    for (size_t i = 0; i < len; ++i) {
        TokenType op = operators_before[i];
        assert(op == SIZEOF || op == INC_OP || op == DEC_OP);
    }
    res->len = len;
    res->operators_before = operators_before;
}

UnaryExpr* create_unary_expr_postfix(TokenType* operators_before, size_t len, PostfixExpr* postfix) {
    assert(postfix);
    UnaryExpr* res = xmalloc(sizeof(UnaryExpr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_POSTFIX;
    res->postfix = postfix;
    
    return res;
}

UnaryExpr* create_unary_expr_unary_op(TokenType* operators_before, size_t len, TokenType unary_op, CastExpr* cast_expr) {
    assert(is_unary_op(unary_op));
    assert(cast_expr);
    UnaryExpr* res = xmalloc(sizeof(UnaryExpr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_UNARY_OP;
    res->unary_op = unary_op;
    res->cast_expr = cast_expr;
    
    return res;
}

UnaryExpr* create_unary_expr_sizeof_type(TokenType* operators_before, size_t len, TypeName* type_name) {
    assert(type_name);
    UnaryExpr* res = xmalloc(sizeof(UnaryExpr));
    assign_operators_before(res, operators_before, len);
    res->type = UNARY_SIZEOF_TYPE;
    res->type_name = type_name;
    
    return res;
}

void free_unary_expr_children(UnaryExpr* u) {
    free(u->operators_before);
    switch (u->type) {
        case UNARY_POSTFIX:
            free_postfix_expr(u->postfix);
            break;
        case UNARY_UNARY_OP:
            free_cast_expr(u->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
            free_type_name(u->type_name);
            break;
    }
}

void free_unary_expr(UnaryExpr* u) {
    free_unary_expr_children(u);
    free(u);
}

