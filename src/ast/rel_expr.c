#include "ast/rel_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct rel_expr* create_rel_expr(struct shift_expr* lhs, struct shift_expr_and_op* rel_chain, size_t len) {
    assert(lhs);
    if (len > 0) {
        assert(rel_chain);
    } else {
        assert(rel_chain == NULL);
    }
    
    for (size_t i = 0; i < len; ++i) {
        assert(rel_chain[i].rhs);
        assert(is_rel_op(rel_chain[i].rel_op));
    }

    struct rel_expr* res = xmalloc(sizeof(struct rel_expr));
    res->lhs = lhs;
    res->len = len;
    res->rel_chain = rel_chain;
    
    return res;
}

void free_rel_expr_children(struct rel_expr* e) {
    free_shift_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_shift_expr(e->rel_chain[i].rhs);
    }
    free(e->rel_chain);
} 

void free_rel_expr(struct rel_expr* e) {
    free_rel_expr_children(e);
    free(e);
}

