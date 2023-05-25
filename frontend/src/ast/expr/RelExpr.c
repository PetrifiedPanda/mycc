#include "frontend/ast/expr/RelExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool is_rel_op(TokenKind k) {
    switch (k) {
        case TOKEN_LE:
        case TOKEN_GE:
        case TOKEN_LT:
        case TOKEN_GT:
            return true;
        default:
            return false;
    }
}

static RelExprOp token_type_to_rel_op(TokenKind t) {
    assert(is_rel_op(t));
    switch (t) {
        case TOKEN_LT:
            return REL_EXPR_LT;
        case TOKEN_GT:
            return REL_EXPR_GT;
        case TOKEN_LE:
            return REL_EXPR_LE;
        case TOKEN_GE:
            return REL_EXPR_GE;

        default:
            UNREACHABLE();
    }
}

static bool parse_rel_expr_rel_chain(ParserState* s, RelExpr* res) {
    assert(res->lhs);

    res->len = 0;
    res->rel_chain = NULL;

    size_t alloc_len = res->len;
    while (is_rel_op(s->it->kind)) {
        const TokenKind op = s->it->kind;
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->rel_chain,
                       &alloc_len,
                       sizeof *res->rel_chain);
        }

        ShiftExprAndOp* curr = &res->rel_chain[res->len];
        curr->rhs = parse_shift_expr(s);
        if (!curr->rhs) {
            goto fail;
        }
        curr->op = token_type_to_rel_op(op);

        ++res->len;
    }

    res->rel_chain = mycc_realloc(res->rel_chain,
                              sizeof *res->rel_chain * res->len);

    return true;
fail:
    free_rel_expr(res);
    return false;
}

struct RelExpr* parse_rel_expr(ParserState* s) {
    ShiftExpr* lhs = parse_shift_expr(s);
    if (!lhs) {
        return NULL;
    }

    struct RelExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_rel_expr_rel_chain(s, res)) {
        return NULL;
    }

    return res;
}

struct RelExpr* parse_rel_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    ShiftExpr* lhs = parse_shift_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    struct RelExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_rel_expr_rel_chain(s, res)) {
        return NULL;
    }

    return res;
}

void free_rel_expr_children(struct RelExpr* e) {
    free_shift_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_shift_expr(e->rel_chain[i].rhs);
    }
    mycc_free(e->rel_chain);
}

void free_rel_expr(struct RelExpr* e) {
    free_rel_expr_children(e);
    mycc_free(e);
}
