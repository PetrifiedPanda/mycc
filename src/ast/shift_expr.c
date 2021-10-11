#include "ast/shift_expr.h"

#include <stdlib.h>
#include <assert.h>

ShiftExpr* create_shift_expr(AddExprAndOp* shift_chain, size_t len) {
    assert(len > 0);
    assert(shift_chain);
    assert(false); // TODO: assert ops
    ShiftExpr* res = malloc(sizeof(ShiftExpr));
    if (res) {
        res->len = len;
        res->shift_chain = shift_chain;
    }
    return res;
}

static void free_children(ShiftExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_add_expr(e->shift_chain[i].add_expr);
    }
    free(e->shift_chain);
}

void free_shift_expr(ShiftExpr* e) {
    free_children(e);
    free(e);
}