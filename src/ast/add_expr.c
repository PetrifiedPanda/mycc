#include "ast/add_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct add_expr* create_add_expr(struct mul_expr* lhs, size_t len, struct mul_expr_and_op* add_chain) {
    assert(lhs); 
    if (len != 0) {
        assert(add_chain);
    } else {
        assert(add_chain == NULL);
    }

    for (size_t i = 0; i < len; ++i) {
        struct mul_expr_and_op* item = &add_chain[i];
        assert(item->rhs);
        assert(is_add_op(item->add_op));
    }
     
    struct add_expr* res = xmalloc(sizeof(struct add_expr));
    res->lhs = lhs;
    res->len = len;
    res->add_chain = add_chain;
    return res;
}

static void free_children(struct add_expr* e) {
    free_mul_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_mul_expr(e->add_chain[i].rhs);
    }
    free(e->add_chain);
}

void free_add_expr(struct add_expr* e) {
    free_children(e);
    free(e);
}

