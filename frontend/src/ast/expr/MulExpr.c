#include "frontend/ast/expr/MulExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool is_mul_op(TokenKind k) {
    switch (k) {
        case TOKEN_ASTERISK:
        case TOKEN_DIV:
        case TOKEN_MOD:
            return true;
        default:
            return false;
    }
}

static MulExprOp token_type_to_mul_op(TokenKind t) {
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

static bool parse_mul_expr_mul_chain(ParserState* s, MulExpr* res) {
    res->len = 0;
    res->mul_chain = NULL;

    size_t alloc_len = res->len;
    while (is_mul_op(s->it->kind)) {
        const TokenKind op = s->it->kind;
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->mul_chain,
                            &alloc_len,
                            sizeof *res->mul_chain);
        }

        CastExprAndOp* curr = &res->mul_chain[res->len];
        curr->rhs = parse_cast_expr(s);
        if (!curr->rhs) {
            MulExpr_free(res);
            return false;
        }
        curr->op = token_type_to_mul_op(op);

        ++res->len;
    }

    res->mul_chain = mycc_realloc(res->mul_chain,
                                  sizeof *res->mul_chain * res->len);

    return true;
}

MulExpr* parse_mul_expr(ParserState* s) {
    CastExpr* lhs = parse_cast_expr(s);
    if (!lhs) {
        return NULL;
    }

    MulExpr* res = mycc_alloc(sizeof *res);

    res->lhs = lhs;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
}

MulExpr* parse_mul_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    MulExpr* res = mycc_alloc(sizeof *res);
    res->lhs = start;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return NULL;
    }

    return res;
}

static void free_mul_expr_children(MulExpr* e) {
    CastExpr_free(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        CastExpr_free(e->mul_chain[i].rhs);
    }
    mycc_free(e->mul_chain);
}

void MulExpr_free(MulExpr* e) {
    free_mul_expr_children(e);
    mycc_free(e);
}
