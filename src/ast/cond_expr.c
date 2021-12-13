#include "ast/cond_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

struct cond_expr* create_cond_expr(struct log_or_and_expr* conditionals, size_t len, struct log_or_expr* last_else) {
    assert(last_else);
    if (len > 0) {
        assert(conditionals);
    } else {
        assert(conditionals == NULL);
    }
    struct cond_expr* res = xmalloc(sizeof(struct cond_expr));
    res->conditionals = conditionals;
    res->len = len;
    res->last_else = last_else;
    
    return res;
}

void free_cond_expr_children(struct cond_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        struct log_or_and_expr* item = &e->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    free(e->conditionals);

    free_log_or_expr(e->last_else);
}

void free_cond_expr(struct cond_expr* e) {
    free_cond_expr_children(e);
    free(e);
}

