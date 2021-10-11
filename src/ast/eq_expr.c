#include "ast/eq_expr.h"

#include <stdlib.h>
#include <assert.h>

void create_eq_expr_inplace(EqExpr* res, RelExprAndOp* eq_neq_chain, size_t len, RelExpr* last_item) {
    assert(last_item);
    if (len > 0) {
        assert(eq_neq_chain);
    } else {
        assert(eq_neq_chain == NULL);
    }
    if (res) {
        res->len = len;
        res->eq_neq_chain = eq_neq_chain;
        res->last_item = last_item;
    }
}

EqExpr* create_eq_expr(RelExprAndOp* eq_neq_chain, size_t len, RelExpr* last_item) {
    
    EqExpr* res = malloc(sizeof(EqExpr));
    create_eq_expr_inplace(res, eq_neq_chain, len, last_item);
    return res;
}

void free_eq_expr_children(EqExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_rel_expr_children(e->eq_neq_chain[i].rel_expr);
    }
    free(e->eq_neq_chain);

    free_rel_expr(e->last_item);
}

void free_eq_expr(EqExpr* e) {
    free_eq_expr_children(e);
    free(e);
}