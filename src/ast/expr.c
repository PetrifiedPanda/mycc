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

}

void free_expr(Expr* e) {
    free_children(e);
    free(e);  
}
