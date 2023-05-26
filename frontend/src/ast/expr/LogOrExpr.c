#include "frontend/ast/expr/LogOrExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static void free_log_or_expr_children(LogOrExpr* e);

static bool parse_log_or_expr_ops(ParserState* s, LogOrExpr* res) {
    assert(res);
    assert(res->len == 1);

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_LOR) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->log_ands,
                            &alloc_len,
                            sizeof *res->log_ands);
        }

        if (!parse_log_and_expr_inplace(s, &res->log_ands[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->log_ands = mycc_realloc(res->log_ands,
                                 sizeof *res->log_ands * res->len);

    return true;
fail:
    free_log_or_expr_children(res);
    return false;
}

LogOrExpr* parse_log_or_expr(ParserState* s) {
    LogAndExpr* and_exprs = mycc_alloc(sizeof *and_exprs);
    if (!parse_log_and_expr_inplace(s, and_exprs)) {
        mycc_free(and_exprs);
        return NULL;
    }

    LogOrExpr* res = mycc_alloc(sizeof *res);
    res->log_ands = and_exprs;
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

LogOrExpr* parse_log_or_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    LogAndExpr* and_exprs = parse_log_and_expr_cast(s, start);
    if (!and_exprs) {
        return NULL;
    }

    LogOrExpr* res = mycc_alloc(sizeof *res);
    res->log_ands = and_exprs;
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

static void free_log_or_expr_children(LogOrExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        LogAndExpr_free_children(&e->log_ands[i]);
    }
    mycc_free(e->log_ands);
}

void LogOrExpr_free(LogOrExpr* e) {
    free_log_or_expr_children(e);
    mycc_free(e);
}

