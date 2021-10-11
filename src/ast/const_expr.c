#include "ast/const_expr.h"

#include <stdlib.h>
#include <assert.h>

ConstExpr* create_const_expr(CondExpr* expr) {
    assert(expr);
    ConstExpr* res = malloc(sizeof(ConstExpr));
    if (res) {
        res->expr = expr;
    }
    return res;
}

static void free_children(ConstExpr* e) {
    free_cond_expr(e->expr);
}

void free_const_expr(ConstExpr* e) {
    free_children(e);
    free(e);
}