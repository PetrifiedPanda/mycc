#include "ast/or_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_or_expr_inplace(struct parser_state* s, struct or_expr* res) {
    res->xor_exprs = xmalloc(sizeof(struct xor_expr));
    if (!parse_xor_expr_inplace(s, res->xor_exprs)) {
        free(res->xor_exprs);
        return false;
    }

    size_t alloc_len = res->len = 1;

    while (s->it->type == OR) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->xor_exprs, &alloc_len, sizeof(struct xor_expr));
        }

        if (!parse_xor_expr_inplace(s, &res->xor_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->xor_exprs = xrealloc(res->xor_exprs, sizeof(struct xor_expr) * res->len);

    return true;

fail:
    free_or_expr_children(res);
    return false;
}

struct or_expr* parse_or_expr_unary(struct parser_state* s, struct unary_expr* start) {
    (void)s;
    (void)start;
    // TODO:
    return NULL;
}

void free_or_expr_children(struct or_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_xor_expr_children(&e->xor_exprs[i]);
    }
    free(e->xor_exprs);
}

void free_or_expr(struct or_expr* e) {
    free_or_expr_children(e);
    free(e);
}

