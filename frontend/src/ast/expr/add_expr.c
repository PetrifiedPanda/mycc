#include "frontend/ast/expr/add_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_add_expr_add_chain(struct parser_state* s,
                                     struct add_expr* res) {
    res->len = 0;
    res->add_chain = NULL;

    size_t alloc_len = res->len;
    while (is_add_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->add_chain,
                       &alloc_len,
                       sizeof(struct mul_expr_and_op));
        }

        struct mul_expr_and_op* curr = &res->add_chain[res->len];
        curr->rhs = parse_mul_expr(s);
        if (!curr->rhs) {
            free_add_expr(res);
            return false;
        }

        curr->add_op = op;

        ++res->len;
    }

    res->add_chain = xrealloc(res->add_chain,
                              sizeof(struct mul_expr_and_op) * res->len);

    return true;
}

struct add_expr* parse_add_expr(struct parser_state* s) {
    struct mul_expr* lhs = parse_mul_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct add_expr* res = xmalloc(sizeof(struct add_expr));
    res->lhs = lhs;

    if (!parse_add_expr_add_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct add_expr* parse_add_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct mul_expr* lhs = parse_mul_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct add_expr* res = xmalloc(sizeof(struct add_expr));
    res->lhs = lhs;

    if (!parse_add_expr_add_chain(s, res)) {
        return NULL;
    }

    return res;
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
