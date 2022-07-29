#include "frontend/ast/expr/mul_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_mul_expr_mul_chain(struct parser_state* s,
                                     struct mul_expr* res) {
    res->len = 0;
    res->mul_chain = NULL;

    size_t alloc_len = res->len;
    while (is_mul_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->mul_chain,
                       &alloc_len,
                       sizeof(struct cast_expr_and_op));
        }

        struct cast_expr_and_op* curr = &res->mul_chain[res->len];
        curr->rhs = parse_cast_expr(s);
        if (!curr->rhs) {
            free_mul_expr(res);
            return false;
        }
        curr->mul_op = op;

        ++res->len;
    }

    res->mul_chain = xrealloc(res->mul_chain,
                              sizeof(struct mul_expr_and_op) * res->len);

    return true;
}

struct mul_expr* parse_mul_expr(struct parser_state* s) {
    struct cast_expr* lhs = parse_cast_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct mul_expr* res = xmalloc(sizeof(struct mul_expr));

    res->lhs = lhs;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct mul_expr* parse_mul_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct mul_expr* res = xmalloc(sizeof(struct mul_expr));
    res->lhs = start;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
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
