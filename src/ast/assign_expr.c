#include "ast/assign_expr.h"

#include <assert.h>
#include <stdlib.h>

void create_assign_expr_inplace(AssignExpr* res, UnaryAndOp* assign_chain, size_t len, CondExpr* value) {
    assert(value);
    if (len > 0) {
        assert(assign_chain);
    } else {
        assert(assign_chain == NULL);
    }
    for (size_t i = 0; i < len; ++i) {
        assert(is_assign_op(assign_chain[i].assign_op));
    }
    if (res) {
        res->assign_chain = assign_chain;
        res->len = len;
        res->value = value;
    }
}

AssignExpr* create_assign_expr(UnaryAndOp* assign_chain, size_t len, CondExpr* value) {
    AssignExpr* res = malloc(sizeof(AssignExpr));
    create_assign_expr_inplace(res, assign_chain, len, value);
    return res;
}

void free_assign_expr_children(AssignExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_unary_expr_children(e->assign_chain[i].unary);
    }
    free(e->assign_chain);
    
    free_cond_expr(e->value);
}

void free_assign_expr(AssignExpr* e) {
    free_assign_expr_children(e);
    free(e);
}

