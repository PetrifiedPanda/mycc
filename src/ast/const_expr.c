#include "ast/const_expr.h"

#include <stdlib.h>

#include "util.h"

static struct const_expr* create_const_expr(struct cond_expr expr) {
    struct const_expr* res = xmalloc(sizeof(struct const_expr));
    res->expr = expr;
    return res;
}

struct const_expr* parse_const_expr(struct parser_state* s) {
    struct const_expr* res = xmalloc(sizeof(struct const_expr));
    if (!parse_cond_expr_inplace(s, &res->expr)) {
        free(res);
        return NULL;
    }
    return res;
}

static void free_children(struct const_expr* e) {
    free_cond_expr_children(&e->expr);
}

void free_const_expr(struct const_expr* e) {
    free_children(e);
    free(e);
}

