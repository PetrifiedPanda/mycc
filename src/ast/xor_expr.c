#include "ast/xor_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct xor_expr* create_xor_expr(struct and_expr* and_exprs, size_t len) {
    assert(len > 0);
    assert(and_exprs);
    struct xor_expr* res = xmalloc(sizeof(struct xor_expr));
    res->len = len;
    res->and_exprs = and_exprs;
    
    return res;
}

void free_xor_expr_children(struct xor_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_and_expr_children(&e->and_exprs[i]);
    }
    free(e->and_exprs);
}

void free_xor_expr(struct xor_expr* e) {
    free_xor_expr_children(e);
    free(e);
}

