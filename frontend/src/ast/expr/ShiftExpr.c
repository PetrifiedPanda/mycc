#include "frontend/ast/expr/ShiftExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool is_shift_op(TokenKind t) {
    switch (t) {
        case TOKEN_LSHIFT:
        case TOKEN_RSHIFT:
            return true;
        default:
            return false;
    }
}

static bool parse_shift_expr_shift_chain(ParserState* s, ShiftExpr* res) {
    res->len = 0;
    res->shift_chain = NULL;

    size_t alloc_len = res->len;
    while (is_shift_op(s->it->kind)) {
        const TokenKind op = s->it->kind;
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->shift_chain,
                            &alloc_len,
                            sizeof *res->shift_chain);
        }

        AddExprAndOp* curr = &res->shift_chain[res->len];
        curr->rhs = parse_add_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = op == TOKEN_LSHIFT ? SHIFT_EXPR_LEFT : SHIFT_EXPR_RIGHT;

        ++res->len;
    }

    res->shift_chain = mycc_realloc(res->shift_chain,
                                    sizeof *res->shift_chain * res->len);

    return true;

fail:
    free_shift_expr(res);
    return false;
}

struct ShiftExpr* parse_shift_expr(ParserState* s) {
    AddExpr* lhs = parse_add_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct ShiftExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_shift_expr_shift_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct ShiftExpr* parse_shift_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    AddExpr* lhs = parse_add_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct ShiftExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_shift_expr_shift_chain(s, res)) {
        return NULL;
    }

    return res;
}

static void free_shift_expr_children(struct ShiftExpr* e) {
    free_add_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_add_expr(e->shift_chain[i].rhs);
    }
    mycc_free(e->shift_chain);
}

void free_shift_expr(struct ShiftExpr* e) {
    free_shift_expr_children(e);
    mycc_free(e);
}
