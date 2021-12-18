#include "ast/and_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res) {
    res->eq_exprs = xmalloc(sizeof(struct eq_expr));
    if (!parse_eq_expr_inplace(s, res->eq_exprs)) {
        free(res->eq_exprs);
        return false;
    }

    size_t alloc_size = res->len = 1;

    while (s->it->type == AND) {
        accept_it(s);
        if (res->len == alloc_size) {
            grow_alloc((void**)&res->eq_exprs, &alloc_size, sizeof(struct eq_expr));
        }
        if (!parse_eq_expr_inplace(s, &res->eq_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->eq_exprs = xrealloc(res->eq_exprs, sizeof(struct eq_expr) * res->len);

    return true;
fail:
    free_and_expr_children(res);
    return false;
}

void free_and_expr_children(struct and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_eq_expr_children(&e->eq_exprs[i]);
    }
    free(e->eq_exprs);
}

void free_and_expr(struct and_expr* e) {
    free_and_expr_children(e);
    free(e);
}

