#include "ast/expr.h"

#include <stdlib.h>
#include <assert.h>

Expr* create_expr(AssignExpr* assign_exprs, size_t len) {
    assert(len > 0);
    assert(assign_exprs);
    Expr* res = malloc(sizeof(Expr));
    if (res) {
        res->len = len;
        res->assign_exprs = assign_exprs;
    }
    return res;
}

static void free_children(Expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_assign_expr_children(&e->assign_exprs[i]);
    }
    free(e->assign_exprs);
}

void free_expr(Expr* e) {
    free_children(e);
    free(e);  
}

