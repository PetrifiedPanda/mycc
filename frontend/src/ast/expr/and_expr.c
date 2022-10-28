#include "frontend/ast/expr/and_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_and_expr_rest(struct parser_state* s, struct and_expr* res) {
    assert(res->eq_exprs);

    res->len = 1;
    size_t alloc_len = res->len;

    while (s->it->type == AND) {
        accept_it(s);
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->eq_exprs, &alloc_len, sizeof *res->eq_exprs);
        }
        if (!parse_eq_expr_inplace(s, &res->eq_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->eq_exprs = xrealloc(res->eq_exprs, sizeof *res->eq_exprs * res->len);

    return true;
fail:
    free_and_expr_children(res);
    return false;
}

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res) {
    res->eq_exprs = xmalloc(sizeof *res->eq_exprs);
    if (!parse_eq_expr_inplace(s, res->eq_exprs)) {
        free(res->eq_exprs);
        return false;
    }

    if (!parse_and_expr_rest(s, res)) {
        return false;
    }
    return true;
}

struct and_expr* parse_and_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct eq_expr* eq_exprs = parse_eq_expr_cast(s, start);
    if (!eq_exprs) {
        return NULL;
    }

    struct and_expr* res = xmalloc(sizeof *res);
    res->eq_exprs = eq_exprs;

    if (!parse_and_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

void free_and_expr_children(struct and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_eq_expr_children(&e->eq_exprs[i]);
    }
    free(e->eq_exprs);
}
