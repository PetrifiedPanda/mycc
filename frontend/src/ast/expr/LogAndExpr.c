#include "frontend/ast/expr/LogAndExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_log_and_expr_rest(ParserState* s, LogAndExpr* res) {
    assert(res);
    res->len = 1;
    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_LAND) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->or_exprs,
                       &alloc_len,
                       sizeof *res->or_exprs);
        }

        if (!parse_or_expr_inplace(s, &res->or_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->or_exprs = mycc_realloc(res->or_exprs, sizeof *res->or_exprs * res->len);
    return true;
fail:
    free_log_and_expr_children(res);
    return false;
}

bool parse_log_and_expr_inplace(ParserState* s, LogAndExpr* res) {
    assert(res);

    res->or_exprs = mycc_alloc(sizeof *res->or_exprs);
    if (!parse_or_expr_inplace(s, res->or_exprs)) {
        mycc_free(res->or_exprs);
        return false;
    }

    if (!parse_log_and_expr_rest(s, res)) {
        return false;
    }

    return true;
}

LogAndExpr* parse_log_and_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    OrExpr* or_exprs = parse_or_expr_cast(s, start);
    if (!or_exprs) {
        return NULL;
    }

    LogAndExpr* res = mycc_alloc(sizeof *res);
    res->or_exprs = or_exprs;

    if (!parse_log_and_expr_rest(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

void free_log_and_expr_children(LogAndExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_or_expr_children(&e->or_exprs[i]);
    }
    mycc_free(e->or_exprs);
}
