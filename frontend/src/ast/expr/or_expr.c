#include "frontend/ast/expr/or_expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_or_expr_rest(struct parser_state* s, struct or_expr* res) {
    res->len = 1;

    size_t alloc_len = res->len;
    while (s->it->kind == OR) {
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->xor_exprs,
                       &alloc_len,
                       sizeof *res->xor_exprs);
        }

        if (!parse_xor_expr_inplace(s, &res->xor_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->xor_exprs = mycc_realloc(res->xor_exprs,
                              sizeof *res->xor_exprs * res->len);

    return true;

fail:
    free_or_expr_children(res);
    return false;
}

bool parse_or_expr_inplace(struct parser_state* s, struct or_expr* res) {
    assert(res);

    res->xor_exprs = mycc_alloc(sizeof *res->xor_exprs);
    if (!parse_xor_expr_inplace(s, res->xor_exprs)) {
        mycc_free(res->xor_exprs);
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

    struct or_expr* res = mycc_alloc(sizeof *res);
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
    mycc_free(e->xor_exprs);
}
