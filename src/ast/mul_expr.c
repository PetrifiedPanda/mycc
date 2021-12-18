#include "ast/mul_expr.h"

#include <stdlib.h>

#include "util.h"

#include "parser/parser_util.h"

struct mul_expr* parse_mul_expr(struct parser_state* s) {
    struct cast_expr* lhs = parse_cast_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct mul_expr* res = xmalloc(sizeof(struct mul_expr));

    res->lhs = lhs;

    size_t alloc_size = res->len = 0;
    res->mul_chain = NULL;

    while (is_mul_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_size) {
            grow_alloc((void**)&res->mul_chain, &alloc_size, sizeof(struct cast_expr_and_op));
        }

        struct cast_expr_and_op* curr = &res->mul_chain[res->len];
        curr->rhs = parse_cast_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->mul_op = op;

        ++res->len;
    }

    res->mul_chain = xrealloc(res->mul_chain, sizeof(struct mul_expr_and_op) * res->len);

    return res;

fail:
    free_mul_expr(res);
    return NULL;
}

static void free_children(struct mul_expr* e) {
    free_cast_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_cast_expr(e->mul_chain[i].rhs);
    }
    free(e->mul_chain);
}

void free_mul_expr(struct mul_expr* e) {
    free_children(e);
    free(e);
}

