#include "ast/and_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct and_expr* create_and_expr(struct eq_expr* eq_exprs, size_t len) {
    assert(eq_exprs);
    assert(len > 0);
    struct and_expr* res = xmalloc(sizeof(struct and_expr));
    res->eq_exprs = eq_exprs;
    res->len = len;
    return res;
}

void free_and_expr_children(struct and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_eq_expr_children(&e->eq_exprs[i]);
    }
    free(e->eq_exprs);
}

void free_and_expr(struct and_expr* e) {
    free_and_expr_children(e);
    free(e);
}

