#include "frontend/ast/expr/eq_expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool is_eq_op(enum token_kind t) {
    switch (t) {
        case EQ_OP:
        case NE_OP:
            return true;
        default:
            return false;
    }
}

static bool parse_eq_expr_eq_chain(struct parser_state* s,
                                   struct eq_expr* res) {
    assert(res->lhs);

    res->len = 0;
    res->eq_chain = NULL;

    size_t alloc_len = res->len;
    while (is_eq_op(s->it->kind)) {
        enum token_kind op = s->it->kind;
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->eq_chain,
                       &alloc_len,
                       sizeof *res->eq_chain);
        }

        struct rel_expr_and_op* curr = &res->eq_chain[res->len];
        curr->rhs = parse_rel_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = op == EQ_OP ? EQ_EXPR_EQ : EQ_EXPR_NE;

        ++res->len;
    }

    res->eq_chain = mycc_realloc(res->eq_chain, sizeof *res->eq_chain * res->len);

    return true;
fail:
    free_eq_expr_children(res);
    return false;
}

bool parse_eq_expr_inplace(struct parser_state* s, struct eq_expr* res) {
    assert(res);

    res->lhs = parse_rel_expr(s);
    if (!res->lhs) {
        return false;
    }

    if (!parse_eq_expr_eq_chain(s, res)) {
        return false;
    }

    return true;
}

struct eq_expr* parse_eq_expr_cast(struct parser_state* s,
                                   struct cast_expr* start) {
    assert(start);

    struct rel_expr* lhs = parse_rel_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct eq_expr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_eq_expr_eq_chain(s, res)) {
        return NULL;
    }

    return res;
}

void free_eq_expr_children(struct eq_expr* e) {
    free_rel_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_rel_expr(e->eq_chain[i].rhs);
    }
    mycc_free(e->eq_chain);
}
