#include "ast/const_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

ConstExpr* create_const_expr(CondExpr expr) {
    ConstExpr* res = xmalloc(sizeof(ConstExpr));
    res->expr = expr;
    return res;
}

static void free_children(ConstExpr* e) {
    free_cond_expr_children(&e->expr);
}

void free_const_expr(ConstExpr* e) {
    free_children(e);
    free(e);
}

