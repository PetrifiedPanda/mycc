#include "frontend/ast/expr/AndExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_and_expr_rest(ParserState* s, AndExpr* res) {
    assert(res->eq_exprs);

    res->len = 1;
    size_t alloc_len = res->len;

    while (s->it->kind == TOKEN_AND) {
        parser_accept_it(s);
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->eq_exprs, &alloc_len, sizeof *res->eq_exprs);
        }
        if (!parse_eq_expr_inplace(s, &res->eq_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->eq_exprs = mycc_realloc(res->eq_exprs, sizeof *res->eq_exprs * res->len);

    return true;
fail:
    free_and_expr_children(res);
    return false;
}

bool parse_and_expr_inplace(ParserState* s, AndExpr* res) {
    res->eq_exprs = mycc_alloc(sizeof *res->eq_exprs);
    if (!parse_eq_expr_inplace(s, res->eq_exprs)) {
        mycc_free(res->eq_exprs);
        return false;
    }

    if (!parse_and_expr_rest(s, res)) {
        return false;
    }
    return true;
}

AndExpr* parse_and_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    EqExpr* eq_exprs = parse_eq_expr_cast(s, start);
    if (!eq_exprs) {
        return NULL;
    }

    AndExpr* res = mycc_alloc(sizeof *res);
    res->eq_exprs = eq_exprs;

    if (!parse_and_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

void free_and_expr_children(AndExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_eq_expr_children(&e->eq_exprs[i]);
    }
    mycc_free(e->eq_exprs);
}
