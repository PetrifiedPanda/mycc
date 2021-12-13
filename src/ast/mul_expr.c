#include "ast/mul_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct mul_expr* create_mul_expr(struct cast_expr* lhs, struct cast_expr_and_op* mul_chain, size_t len) {
    assert(lhs);
    if (len != 0) {
        assert(mul_chain);
    } else {
        assert(mul_chain == NULL);
    }
    
    for (size_t i = 0; i < len; ++i) {
        struct cast_expr_and_op* item = &mul_chain[i];
        assert(item->rhs);
        assert(is_mul_op(item->mul_op));
    }

    struct mul_expr* res = xmalloc(sizeof(struct mul_expr));
    res->lhs = lhs;
    res->len = len;
    res->mul_chain = mul_chain;
    
    return res;
}

static void free_children(struct mul_expr* e) {
    free_cast_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_cast_expr(e->mul_chain[i].rhs);
    }
    free(e->mul_chain);
}

void free_mul_expr(struct mul_expr* e) {
    free_children(e);
    free(e);
}

