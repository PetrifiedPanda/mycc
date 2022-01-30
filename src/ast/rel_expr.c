#include "ast/rel_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static bool parse_rel_expr_rel_chain(struct parser_state* s, struct rel_expr* res) {
    assert(res->lhs);

    size_t alloc_len = res->len = 0;
    res->rel_chain = NULL;

    while (is_rel_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->rel_chain, &alloc_len, sizeof(struct shift_expr_and_op));
        }

        struct shift_expr_and_op* curr = &res->rel_chain[res->len];
        curr->rhs = parse_shift_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->rel_op = op;

        ++res->len;
    }

    res->rel_chain = xrealloc(res->rel_chain, sizeof(struct shift_expr_and_op) * res->len);

    return true;
fail:
    free_rel_expr(res);
    return false;
}

struct rel_expr* parse_rel_expr(struct parser_state* s) {
    struct shift_expr* lhs = parse_shift_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct rel_expr* res = xmalloc(sizeof(struct rel_expr));
    res->lhs = lhs;

    if (!parse_rel_expr_rel_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct rel_expr* parse_rel_expr_cast(struct parser_state* s, struct cast_expr* start) {
    assert(start);

    struct shift_expr* lhs = parse_shift_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct rel_expr* res = xmalloc(sizeof(struct rel_expr));
    res->lhs = lhs;

    if (!parse_rel_expr_rel_chain(s, res)) {
        return NULL;
    }

    return res;
}

void free_rel_expr_children(struct rel_expr* e) {
    free_shift_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_shift_expr(e->rel_chain[i].rhs);
    }
    free(e->rel_chain);
} 

void free_rel_expr(struct rel_expr* e) {
    free_rel_expr_children(e);
    free(e);
}

