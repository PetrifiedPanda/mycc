#include "ast/shift_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct shift_expr* create_shift_expr(struct add_expr* lhs, struct add_expr_and_op* shift_chain, size_t len) {
    assert(lhs);
    if (len > 0) {
        assert(shift_chain);
    } else {
        assert(shift_chain == NULL);
    }

    for (size_t i = 0; i < len; ++i) {
        assert(shift_chain[i].rhs);
        assert(is_shift_op(shift_chain[i].shift_op));
    }
    
    struct shift_expr* res = xmalloc(sizeof(struct shift_expr));
    res->lhs = lhs;
    res->len = len;
    res->shift_chain = shift_chain;
    
    return res;
}

static void free_children(struct shift_expr* e) {
    free_add_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_add_expr(e->shift_chain[i].rhs);
    }
    free(e->shift_chain);
}

void free_shift_expr(struct shift_expr* e) {
    free_children(e);
    free(e);
}

