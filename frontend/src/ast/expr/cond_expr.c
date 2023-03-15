#include "frontend/ast/expr/cond_expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_cond_expr_conditionals(struct parser_state* s,
                                         struct cond_expr* res) {
    res->len = 0;
    res->conditionals = NULL;

    size_t alloc_len = res->len;
    while (s->it->kind == QMARK) {
        accept_it(s);

        struct expr* expr = parse_expr(s);
        if (!expr) {
            goto fail;
        }

        if (!accept(s, COLON)) {
            free_expr(expr);
            goto fail;
        }

        struct log_or_expr* new_last = parse_log_or_expr(s);
        if (!new_last) {
            free_expr(expr);
            goto fail;
        }

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->conditionals, &alloc_len, sizeof *res->conditionals);
        }

        res->conditionals[res->len] = (struct log_or_and_expr){
            .log_or = res->last_else,
            .expr = expr,
        };

        res->last_else = new_last;

        ++res->len;
    }

    res->conditionals = mycc_realloc(res->conditionals, sizeof *res->conditionals * res->len);
    return true;

fail:
    free_cond_expr_children(res);
    return false;
}

bool parse_cond_expr_inplace(struct parser_state* s, struct cond_expr* res) {
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

struct cond_expr* parse_cond_expr_cast(struct parser_state* s,
                                       struct cast_expr* start) {
    assert(start);

    struct cond_expr* res = mycc_alloc(sizeof *res);
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

void free_cond_expr_children(struct cond_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        struct log_or_and_expr* item = &e->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    mycc_free(e->conditionals);

    free_log_or_expr(e->last_else);
}

void free_cond_expr(struct cond_expr* e) {
    free_cond_expr_children(e);
    mycc_free(e);
}

