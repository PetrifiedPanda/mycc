#include "frontend/ast/expr/AddExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool is_add_op(TokenKind k) {
    switch (k) {
        case TOKEN_ADD:
        case TOKEN_SUB:
            return true;
        default:
            return false;
    }
}

static bool parse_add_expr_add_chain(ParserState* s, AddExpr* res) {
    res->len = 0;
    res->add_chain = NULL;

    size_t alloc_len = res->len;
    while (is_add_op(s->it->kind)) {
        const TokenKind op = s->it->kind;
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->add_chain,
                            &alloc_len,
                            sizeof *res->add_chain);
        }

        MulExprAndOp* curr = &res->add_chain[res->len];
        curr->rhs = parse_mul_expr(s);
        if (!curr->rhs) {
            free_add_expr(res);
            return false;
        }

        curr->op = op == TOKEN_ADD ? ADD_EXPR_ADD : ADD_EXPR_SUB;

        ++res->len;
    }

    res->add_chain = mycc_realloc(res->add_chain,
                                  sizeof *res->add_chain * res->len);

    return true;
}

AddExpr* parse_add_expr(ParserState* s) {
    MulExpr* lhs = parse_mul_expr(s);
    if (!lhs) {
        return NULL;
    }

    AddExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_add_expr_add_chain(s, res)) {
        return NULL;
    }

    return res;
}

AddExpr* parse_add_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    MulExpr* lhs = parse_mul_expr_cast(s, start);
    if (!lhs) {
        return NULL;
    }

    AddExpr* res = mycc_alloc(sizeof *res);
    res->lhs = lhs;

    if (!parse_add_expr_add_chain(s, res)) {
        return NULL;
    }

    return res;
}

static void free_add_expr_children(AddExpr* e) {
    free_mul_expr(e->lhs);
    for (size_t i = 0; i < e->len; ++i) {
        free_mul_expr(e->add_chain[i].rhs);
    }
    mycc_free(e->add_chain);
}

void free_add_expr(AddExpr* e) {
    free_add_expr_children(e);
    mycc_free(e);
}
