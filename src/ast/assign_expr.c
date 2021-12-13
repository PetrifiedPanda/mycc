#include "ast/assign_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

void init_assign_expr(struct assign_expr* res, struct unary_and_op* assign_chain, size_t len, struct cond_expr* value) {
    assert(value);
    if (len > 0) {
        assert(assign_chain);
    } else {
        assert(assign_chain == NULL);
    }
    for (size_t i = 0; i < len; ++i) {
        assert(is_assign_op(assign_chain[i].assign_op));
    }

    res->assign_chain = assign_chain;
    res->len = len;
    res->value = value;
}

struct assign_expr* create_assign_expr(struct unary_and_op* assign_chain, size_t len, struct cond_expr* value) {
    struct assign_expr* res = xmalloc(sizeof(struct assign_expr));
    init_assign_expr(res, assign_chain, len, value);
    return res;
}

void free_assign_expr_children(struct assign_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_unary_expr_children(e->assign_chain[i].unary);
    }
    free(e->assign_chain);
    
    free_cond_expr(e->value);
}

void free_assign_expr(struct assign_expr* e) {
    free_assign_expr_children(e);
    free(e);
}

