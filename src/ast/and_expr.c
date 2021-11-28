#include "ast/and_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

AndExpr* create_and_expr(EqExpr* eq_exprs, size_t len) {
    assert(eq_exprs);
    assert(len > 0);
    AndExpr* res = xmalloc(sizeof(AndExpr));
    res->eq_exprs = eq_exprs;
    res->len = len;
    return res;
}

void free_and_expr_children(AndExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_eq_expr_children(&e->eq_exprs[i]);
    }
    free(e->eq_exprs);
}

void free_and_expr(AndExpr* e) {
    free_and_expr_children(e);
    free(e);
}

