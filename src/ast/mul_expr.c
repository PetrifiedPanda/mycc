#include "ast/mul_expr.h"

#include <stdlib.h>
#include <assert.h>

MulExpr* create_mul_expr(CastExprAndOp* mul_chain, size_t len) {
    assert(len > 0);
    assert(mul_chain);
    assert(false); // TODO: assert ops
    MulExpr* res = malloc(sizeof(MulExpr));
    if (res) {
        res->len = len;
        res->mul_chain = mul_chain;
    }
    return res;
}

static void free_children(MulExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_cast_expr(e->mul_chain[i].cast_expr);
    }
    free(e->mul_chain);
}

void free_mul_expr(MulExpr* e) {
    free_children(e);
    free(e);
}