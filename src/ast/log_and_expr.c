#include "ast/log_and_expr.h"

#include <assert.h>
#include <stdlib.h>

LogAndExpr* create_log_and_expr(OrExpr* or_exprs, size_t len) {
    assert(len > 0);
    assert(or_exprs);
    LogAndExpr* res = malloc(sizeof(LogAndExpr));
    if (res) {
        res->len = len;
        res->or_exprs = or_exprs;
    }
    return res;
}

static void free_children(LogAndExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_or_expr_children(&e->or_exprs[i]);
    }
    free(e->or_exprs);
}

void free_log_and_expr(LogAndExpr* e) {
    free_children(e);
    free(e);
}

