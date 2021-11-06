#include "ast/eq_expr.h"

#include <stdlib.h>
#include <assert.h>

void init_eq_expr(EqExpr* res, RelExpr* lhs, RelExprAndOp* eq_chain, size_t len) {
    assert(lhs);
    if (len > 0) {
        assert(eq_chain);
    } else {
        assert(eq_chain == NULL);
    }

    for (size_t i = 0; i < len; ++i) {
        RelExprAndOp* item = &eq_chain[i];
        assert(item->rhs);
        assert(is_eq_op(item->eq_op));
    }

    if (res) {
        res->lhs = lhs;
        res->len = len;
        res->eq_chain = eq_chain;
    }
}

EqExpr* create_eq_expr(RelExpr* lhs, RelExprAndOp* eq_chain, size_t len) { 
    EqExpr* res = malloc(sizeof(EqExpr));
    init_eq_expr(res, lhs, eq_chain, len);
    return res;
}

void free_eq_expr_children(EqExpr* e) {
    free_rel_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_rel_expr_children(e->eq_chain[i].rhs);
    }
    free(e->eq_chain);
}

void free_eq_expr(EqExpr* e) {
    free_eq_expr_children(e);
    free(e);
}

