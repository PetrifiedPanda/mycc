#include "ast/log_or_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct log_or_expr* create_log_or_expr(size_t len, struct log_and_expr* log_and) {
    assert(len > 0);
    assert(log_and);
    struct log_or_expr* res = xmalloc(sizeof(struct log_or_expr));
    res->len = len;
    res->log_ands = log_and;
    
    return res;
}

static void free_children(struct log_or_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_log_and_expr_children(&e->log_ands[i]);
    }
    free(e->log_ands);
}

void free_log_or_expr(struct log_or_expr* e) {
    free_children(e);
    free(e);
}

