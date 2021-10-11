#include "ast/or_expr.h"

#include <stdlib.h>
#include <assert.h>

OrExpr* create_or_expr(XorExpr* xor_exprs, size_t len) {
    assert(len > 0);
    assert(xor_exprs);
    OrExpr* res = malloc(sizeof(OrExpr));
    if (res) {
        res->xor_exprs = xor_exprs;
        res->len = len;
    }
}

void free_or_expr_children(OrExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_xor_expr_children(&e->xor_exprs[i]);
    }
    free(e->xor_exprs);
}

void free_or_expr(OrExpr* e) {
    free_or_expr_children(e);
    free(e);
}