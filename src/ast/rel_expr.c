#include "ast/rel_expr.h"

#include <stdlib.h>
#include <assert.h>

RelExpr* create_rel_expr(ShiftExprAndOp* rel_chain, size_t len) {
    assert(len > 0);
    assert(rel_chain);
    if (len == 1) {
        assert(rel_chain->rel_op == INVALID);
    }
    assert(false); // TODO: assert ops
    RelExpr* res = malloc(sizeof(RelExpr));
    if (res) {
        res->len = len;
        res->rel_chain = rel_chain;
    }
    return res;
}

void free_rel_expr_children(RelExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_shift_expr(e->rel_chain[i].shift_expr);
    }
    free(e->rel_chain);
} 

void free_rel_expr(RelExpr* e) {
    free_rel_expr_children(e);
    free(e);
}

