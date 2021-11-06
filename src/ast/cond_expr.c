#include "ast/cond_expr.h"

#include <assert.h>
#include <stdlib.h>

CondExpr* create_cond_expr(LogOrAndExpr* conditionals, size_t len, LogOrExpr* last_else) {
    assert(last_else);
    if (len > 0) {
        assert(conditionals);
    } else {
        assert(conditionals == NULL);
    }
    CondExpr* res = malloc(sizeof(CondExpr));
    if (res) {
        res->conditionals = conditionals;
        res->len = len;
        res->last_else = last_else;
    }
    return res;
}

void free_cond_expr_children(CondExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        LogOrAndExpr* item = &e->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    free(e->conditionals);

    free_log_or_expr(e->last_else);
}

void free_cond_expr(CondExpr* e) {
    free_cond_expr_children(e);
    free(e);
}

