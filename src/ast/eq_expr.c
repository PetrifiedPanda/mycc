#include "ast/eq_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

void init_eq_expr(struct eq_expr* res, struct rel_expr* lhs, struct rel_expr_and_op* eq_chain, size_t len) {
    assert(lhs);
    if (len > 0) {
        assert(eq_chain);
    } else {
        assert(eq_chain == NULL);
    }

    for (size_t i = 0; i < len; ++i) {
        assert(eq_chain[i].rhs);
        assert(is_eq_op(eq_chain[i].eq_op));
    }

    res->lhs = lhs;
    res->len = len;
    res->eq_chain = eq_chain;
}

struct eq_expr* create_eq_expr(struct rel_expr* lhs, struct rel_expr_and_op* eq_chain, size_t len) { 
    struct eq_expr* res = xmalloc(sizeof(struct eq_expr));
    init_eq_expr(res, lhs, eq_chain, len);
    return res;
}

void free_eq_expr_children(struct eq_expr* e) {
    free_rel_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_rel_expr_children(e->eq_chain[i].rhs);
    }
    free(e->eq_chain);
}

void free_eq_expr(struct eq_expr* e) {
    free_eq_expr_children(e);
    free(e);
}

