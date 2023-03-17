#include "frontend/ast/expr/mul_expr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool is_mul_op(enum token_kind k) {
    switch (k) {
        case TOKEN_ASTERISK:
        case TOKEN_DIV:
        case TOKEN_MOD:
            return true;
        default:
            return false;
    }
}

static enum mul_expr_op token_type_to_mul_op(enum token_kind t) {
    assert(is_mul_op(t));
    switch (t) {
        case TOKEN_ASTERISK:
            return MUL_EXPR_MUL;
        case TOKEN_DIV:
            return MUL_EXPR_DIV;
        case TOKEN_MOD:
            return MUL_EXPR_MOD;

        default:
            UNREACHABLE();
    }
}

static bool parse_mul_expr_mul_chain(struct parser_state* s,
                                     struct mul_expr* res) {
    res->len = 0;
    res->mul_chain = NULL;

    size_t alloc_len = res->len;
    while (is_mul_op(s->it->kind)) {
        enum token_kind op = s->it->kind;
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->mul_chain, &alloc_len, sizeof *res->mul_chain);
        }

        struct cast_expr_and_op* curr = &res->mul_chain[res->len];
        curr->rhs = parse_cast_expr(s);
        if (!curr->rhs) {
            free_mul_expr(res);
            return false;
        }
        curr->op = token_type_to_mul_op(op);

        ++res->len;
    }

    res->mul_chain = mycc_realloc(res->mul_chain, sizeof *res->mul_chain * res->len);

    return true;
}

struct mul_expr* parse_mul_expr(struct parser_state* s) {
    struct cast_expr* lhs = parse_cast_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct mul_expr* res = mycc_alloc(sizeof *res);
    
    res->lhs = lhs;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct mul_expr* parse_mul_expr_cast(struct parser_state* s,
                                     struct cast_expr* start) {
    assert(start);

    struct mul_expr* res = mycc_alloc(sizeof *res);
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
    mycc_free(e->mul_chain);
}

void free_mul_expr(struct mul_expr* e) {
    free_children(e);
    mycc_free(e);
}
