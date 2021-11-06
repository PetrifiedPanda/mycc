#include "ast/shift_expr.h"

#include <stdlib.h>
#include <assert.h>

ShiftExpr* create_shift_expr(AddExpr* lhs, AddExprAndOp* shift_chain, size_t len) {
    assert(lhs);
    if (len > 0) {
        assert(shift_chain);
    } else {
        assert(shift_chain == NULL);
    }

    for (size_t i = 0; i < len; ++i) {
        AddExprAndOp* item = &shift_chain[i];
        assert(item->rhs);
        assert(is_shift_op(item->shift_op));
    }
    
    ShiftExpr* res = malloc(sizeof(ShiftExpr));
    if (res) {
        res->len = len;
        res->shift_chain = shift_chain;
    }
    return res;
}

static void free_children(ShiftExpr* e) {
    free_add_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_add_expr(e->shift_chain[i].rhs);
    }
    free(e->shift_chain);
}

void free_shift_expr(ShiftExpr* e) {
    free_children(e);
    free(e);
}

