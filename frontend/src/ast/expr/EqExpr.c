#include "frontend/ast/expr/EqExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool is_eq_op(TokenKind t) {
    switch (t) {
        case TOKEN_EQ:
        case TOKEN_NE:
            return true;
        default:
            return false;
    }
}

static bool parse_eq_expr_eq_chain(ParserState* s, EqExpr* res) {
    assert(res->lhs);

    res->len = 0;
    res->eq_chain = NULL;

    size_t alloc_len = res->len;
    while (is_eq_op(s->it->kind)) {
        const TokenKind op = s->it->kind;
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->eq_chain,
                       &alloc_len,
                       sizeof *res->eq_chain);
        }

        RelExprAndOp* curr = &res->eq_chain[res->len];
        curr->rhs = parse_rel_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = op == TOKEN_EQ ? EQ_EXPR_EQ : EQ_EXPR_NE;

        ++res->len;
    }

    res->eq_chain = mycc_realloc(res->eq_chain, sizeof *res->eq_chain * res->len);

    return true;
fail:
    EqExpr_free_children(res);
    return false;
}

bool parse_eq_expr_inplace(ParserState* s, EqExpr* res) {
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

EqExpr* parse_eq_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    RelExpr* lhs = parse_rel_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    EqExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_eq_expr_eq_chain(s, res)) {
        return NULL;
    }

    return res;
}

void EqExpr_free_children(EqExpr* e) {
    RelExpr_free(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        RelExpr_free(e->eq_chain[i].rhs);
    }
    mycc_free(e->eq_chain);
}
