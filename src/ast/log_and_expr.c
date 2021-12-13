#include "ast/log_and_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

struct log_and_expr* create_log_and_expr(struct or_expr* or_exprs, size_t len) {
    assert(len > 0);
    assert(or_exprs);
    struct log_and_expr* res = xmalloc(sizeof(struct log_and_expr));
    res->len = len;
    res->or_exprs = or_exprs;
    
    return res;
}

void free_log_and_expr_children(struct log_and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_or_expr_children(&e->or_exprs[i]);
    }
    free(e->or_exprs);
}

void free_log_and_expr(struct log_and_expr* e) {
    free_log_and_expr_children(e);
    free(e);
}

