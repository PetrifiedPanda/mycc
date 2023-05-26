#include "frontend/ast/expr/CondExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_cond_expr_conditionals(ParserState* s,
                                         CondExpr* res) {
    res->len = 0;
    res->conditionals = NULL;

    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_QMARK) {
        parser_accept_it(s);

        Expr* expr = parse_expr(s);
        if (!expr) {
            goto fail;
        }

        if (!parser_accept(s, TOKEN_COLON)) {
            Expr_free(expr);
            goto fail;
        }

        LogOrExpr* new_last = parse_log_or_expr(s);
        if (!new_last) {
            Expr_free(expr);
            goto fail;
        }

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->conditionals, &alloc_len, sizeof *res->conditionals);
        }

        res->conditionals[res->len] = (LogOrAndExpr){
            .log_or = res->last_else,
            .expr = expr,
        };

        res->last_else = new_last;

        ++res->len;
    }

    res->conditionals = mycc_realloc(res->conditionals, sizeof *res->conditionals * res->len);
    return true;

fail:
    CondExpr_free_children(res);
    return false;
}

bool parse_cond_expr_inplace(ParserState* s, CondExpr* res) {
    assert(res);

    res->last_else = parse_log_or_expr(s);
    if (!res->last_else) {
        return false;
    }

    if (!parse_cond_expr_conditionals(s, res)) {
        return false;
    }

    return true;
}

CondExpr* parse_cond_expr_cast(ParserState* s, CastExpr* start) {
    assert(start);

    CondExpr* res = mycc_alloc(sizeof *res);
    res->last_else = parse_log_or_expr_cast(s, start);
    if (!res->last_else) {
        mycc_free(res);
        return NULL;
    }

    if (!parse_cond_expr_conditionals(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void CondExpr_free_children(CondExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        LogOrAndExpr* item = &e->conditionals[i];
        LogOrExpr_free(item->log_or);
        Expr_free(item->expr);
    }
    mycc_free(e->conditionals);

    LogOrExpr_free(e->last_else);
}

void CondExpr_free(CondExpr* e) {
    CondExpr_free_children(e);
    mycc_free(e);
}

