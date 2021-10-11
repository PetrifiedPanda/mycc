#include "ast/xor_expr.h"

#include <stdlib.h>
#include <assert.h>

XorExpr* create_xor_expr(AndExpr* and_exprs, size_t len) {
    assert(len > 0);
    assert(and_exprs);
    XorExpr* res = malloc(sizeof(XorExpr));
    if (res) {
        res->len = len;
        res->and_exprs = and_exprs;
    }
    return res;
}

void free_xor_expr_children(XorExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_and_expr_children(&e->and_exprs[i]);
    }
    free(e->and_exprs);
}

void free_xor_expr(XorExpr* e) {
    free_xor_expr_children(e);
    free(e);
}