#include "ast/xor_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_xor_expr_inplace(struct parser_state* s, struct xor_expr* res) {
    res->and_exprs = xmalloc(sizeof(struct and_expr));
    if (!parse_and_expr_inplace(s, res->and_exprs)) {
        free(res->and_exprs);
        return false;
    }

    size_t alloc_len = res->len = 1;

    while (s->it->type == XOR) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->and_exprs, &alloc_len, sizeof(struct and_expr));
        }

        if (!parse_and_expr_inplace(s, &res->and_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->and_exprs = xrealloc(res->and_exprs, sizeof(struct and_expr) * res->len);

    return true;

fail:
    free_xor_expr_children(res);
    return false;
}

void free_xor_expr_children(struct xor_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_and_expr_children(&e->and_exprs[i]);
    }
    free(e->and_exprs);
}

void free_xor_expr(struct xor_expr* e) {
    free_xor_expr_children(e);
    free(e);
}

