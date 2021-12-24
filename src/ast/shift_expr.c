#include "ast/shift_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

struct shift_expr* parse_shift_expr(struct parser_state* s) {
    struct add_expr* lhs = parse_add_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct shift_expr* res = xmalloc(sizeof(struct shift_expr));
    res->lhs = lhs;

    size_t alloc_len = res->len = 0;
    res->shift_chain = NULL;

    while (is_shift_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->shift_chain, &alloc_len, sizeof(struct add_expr_and_op));
        }

        struct add_expr_and_op* curr = &res->shift_chain[res->len];
        curr->rhs = parse_add_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->shift_op = op;

        ++res->len;
    }

    res->shift_chain = xrealloc(res->shift_chain, sizeof(struct add_expr_and_op) * res->len);

    return res;

fail:
    free_shift_expr(res);
    return NULL;
}

static void free_children(struct shift_expr* e) {
    free_add_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_add_expr(e->shift_chain[i].rhs);
    }
    free(e->shift_chain);
}

void free_shift_expr(struct shift_expr* e) {
    free_children(e);
    free(e);
}

