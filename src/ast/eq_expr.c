#include "ast/eq_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_eq_expr_inplace(struct parser_state* s, struct eq_expr* res) {
    res->lhs = parse_rel_expr(s);
    if (!res->lhs) {
        return false;
    }

    size_t alloc_size = res->len = 0;
    res->eq_chain = NULL;

    while (is_eq_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_size) {
            grow_alloc((void**)&res->eq_chain, &alloc_size, sizeof(struct rel_expr_and_op));
        }

        struct rel_expr_and_op* curr = &res->eq_chain[res->len];
        curr->rhs = parse_rel_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->eq_op = op;

        ++res->len;
    }

    res->eq_chain = xrealloc(res->eq_chain, sizeof(struct rel_expr_and_op) * res->len);

    return true;
fail:
    free_eq_expr_children(res);
    return false;
}

void free_eq_expr_children(struct eq_expr* e) {
    free_rel_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_rel_expr_children(e->eq_chain[i].rhs);
    }
    free(e->eq_chain);
}

void free_eq_expr(struct eq_expr* e) {
    free_eq_expr_children(e);
    free(e);
}

