#include "ast/mul_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

MulExpr* create_mul_expr(CastExpr* lhs, CastExprAndOp* mul_chain, size_t len) {
    assert(lhs);
    if (len != 0) {
        assert(mul_chain);
    } else {
        assert(mul_chain == NULL);
    }
    
    for (size_t i = 0; i < len; ++i) {
        CastExprAndOp* item = &mul_chain[i];
        assert(item->rhs);
        assert(is_mul_op(item->mul_op));
    }

    MulExpr* res = xmalloc(sizeof(MulExpr));
    res->lhs = lhs;
    res->len = len;
    res->mul_chain = mul_chain;
    
    return res;
}

static void free_children(MulExpr* e) {
    free_cast_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_cast_expr(e->mul_chain[i].rhs);
    }
    free(e->mul_chain);
}

void free_mul_expr(MulExpr* e) {
    free_children(e);
    free(e);
}

