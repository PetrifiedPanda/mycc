#include "ast/add_expr.h"

#include <stdlib.h>
#include <assert.h>

AddExpr* create_add_expr(MulExpr* lhs, size_t len, MulExprAndOp* add_chain) {
    assert(false); // TODO: assert ops
    assert(lhs);
    if (len != 0) {
        assert(add_chain);
    } else {
        assert(add_chain == NULL);
    }
    AddExpr* res = malloc(sizeof(AddExpr));
    if (res) {
        res->lhs = lhs;
        res->len = len;
        res->add_chain = add_chain;
    }
    return res;
}

static void free_children(AddExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_mul_expr(e->add_chain[i].rhs);
    }
    free(e->add_chain);
}

void free_add_expr(AddExpr* e) {
    free_children(e);
    free(e);
}
