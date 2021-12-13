#include "ast/or_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct or_expr* create_or_expr(struct xor_expr* xor_exprs, size_t len) {
    assert(len > 0);
    assert(xor_exprs);
    struct or_expr* res = xmalloc(sizeof(struct or_expr));
    res->xor_exprs = xor_exprs;
    res->len = len;
    
    return res;
}

void free_or_expr_children(struct or_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_xor_expr_children(&e->xor_exprs[i]);
    }
    free(e->xor_exprs);
}

void free_or_expr(struct or_expr* e) {
    free_or_expr_children(e);
    free(e);
}

