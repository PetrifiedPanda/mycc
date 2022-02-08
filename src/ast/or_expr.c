#include "ast/or_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static bool parse_or_expr_rest(struct parser_state* s, struct or_expr* res) {
    size_t alloc_len = res->len = 1;

    while (s->it->type == OR) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->xor_exprs,
                       &alloc_len,
                       sizeof(struct xor_expr));
        }

        if (!parse_xor_expr_inplace(s, &res->xor_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->xor_exprs = xrealloc(res->xor_exprs,
                              sizeof(struct xor_expr) * res->len);

    return true;

fail:
    free_or_expr_children(res);
    return false;
}

bool parse_or_expr_inplace(struct parser_state* s, struct or_expr* res) {
    assert(res);

    res->xor_exprs = xmalloc(sizeof(struct xor_expr));
    if (!parse_xor_expr_inplace(s, res->xor_exprs)) {
        free(res->xor_exprs);
        return false;
    }

    if (!parse_or_expr_rest(s, res)) {
        return false;
    }

    return true;
}

struct or_expr* parse_or_expr_cast(struct parser_state* s,
                                   struct cast_expr* start) {
    assert(start);

    struct xor_expr* xor_exprs = parse_xor_expr_cast(s, start);
    if (!xor_exprs) {
        return NULL;
    }

    struct or_expr* res = xmalloc(sizeof(struct or_expr));
    res->xor_exprs = xor_exprs;

    if (!parse_or_expr_rest(s, res)) {
        return NULL;
    }

    return res;
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
