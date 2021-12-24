#include "ast/add_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

struct add_expr* parse_add_expr(struct parser_state* s) {
    struct mul_expr* lhs = parse_mul_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct add_expr* res = xmalloc(sizeof(struct add_expr));
    res->lhs = lhs;

    size_t alloc_len = res->len = 0;
    res->add_chain = NULL;

    while (is_add_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->add_chain, &alloc_len, sizeof(struct mul_expr_and_op));
        }

        struct mul_expr_and_op* curr = &res->add_chain[res->len];
        curr->rhs = parse_mul_expr(s);
        if (!curr->rhs) {
            goto fail;
        }

        curr->add_op = op;

        ++res->len;
    }

    res->add_chain = xrealloc(res->add_chain, sizeof(struct mul_expr_and_op) * res->len);

    return res;

fail:
    free_add_expr(res);
    return NULL;
}

static void free_children(struct add_expr* e) {
    free_mul_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_mul_expr(e->add_chain[i].rhs);
    }
    free(e->add_chain);
}

void free_add_expr(struct add_expr* e) {
    free_children(e);
    free(e);
}

