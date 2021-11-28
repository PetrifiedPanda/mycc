#include "ast/rel_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

RelExpr* create_rel_expr(ShiftExpr* lhs, ShiftExprAndOp* rel_chain, size_t len) {
    assert(lhs);
    if (len > 0) {
        assert(rel_chain);
    } else {
        assert(rel_chain == NULL);
    }
    
    for (size_t i = 0; i < len; ++i) {
        ShiftExprAndOp* item = &rel_chain[i];
        assert(item->rhs);
        assert(is_rel_op(item->rel_op));
    }

    RelExpr* res = xmalloc(sizeof(RelExpr));
    res->len = len;
    res->rel_chain = rel_chain;
    
    return res;
}

void free_rel_expr_children(RelExpr* e) {
    free_shift_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_shift_expr(e->rel_chain[i].rhs);
    }
    free(e->rel_chain);
} 

void free_rel_expr(RelExpr* e) {
    free_rel_expr_children(e);
    free(e);
}

