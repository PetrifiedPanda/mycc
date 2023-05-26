#include "frontend/ast/expr/XorExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_xor_expr_rest(ParserState* s, XorExpr* res) {
    assert(res->and_exprs);

    res->len = 1;

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_XOR) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->and_exprs, &alloc_len, sizeof *res->and_exprs); 
        }

        if (!parse_and_expr_inplace(s, &res->and_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->and_exprs = mycc_realloc(res->and_exprs, sizeof *res->and_exprs * res->len);
    return true;

fail:
    XorExpr_free_children(res);
    return false;
}

bool parse_xor_expr_inplace(ParserState* s, XorExpr* res) {
    assert(res);

    res->and_exprs = mycc_alloc(sizeof *res->and_exprs);
    if (!parse_and_expr_inplace(s, res->and_exprs)) {
        mycc_free(res->and_exprs);
        return false;
    }

    if (!parse_xor_expr_rest(s, res)) {
        return false;
    }

    return true;
}

XorExpr* parse_xor_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    AndExpr* and_exprs = parse_and_expr_cast(s, start);
    if (!and_exprs) {
        return NULL;
    }

    XorExpr* res = mycc_alloc(sizeof *res);
    res->and_exprs = and_exprs;

    if (!parse_xor_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

void XorExpr_free_children(XorExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        AndExpr_free_children(&e->and_exprs[i]);
    }
    mycc_free(e->and_exprs);
}
