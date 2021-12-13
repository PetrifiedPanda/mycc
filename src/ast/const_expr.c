#include "ast/const_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct const_expr* create_const_expr(struct cond_expr expr) {
    struct const_expr* res = xmalloc(sizeof(struct const_expr));
    res->expr = expr;
    return res;
}

static void free_children(struct const_expr* e) {
    free_cond_expr_children(&e->expr);
}

void free_const_expr(struct const_expr* e) {
    free_children(e);
    free(e);
}

