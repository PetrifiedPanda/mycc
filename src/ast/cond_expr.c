#include "ast/cond_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

#include "parser/parser_util.h"

struct cond_expr* parse_cond_expr(struct parser_state* s) {
    struct cond_expr* res = xmalloc(sizeof(struct cond_expr));
    res->last_else = parse_log_or_expr(s);
    if (!res->last_else) {
        return NULL;
    }

    size_t alloc_len = res->len = 0;
    res->conditionals = NULL;

    while (s->it->type == QMARK) {
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
            grow_alloc((void**)&res->conditionals, &alloc_len, sizeof(struct log_or_and_expr));
        }

        res->conditionals[res->len] = (struct log_or_and_expr){
                .log_or = res->last_else,
                .expr = expr
        };

        res->last_else = new_last;

        ++res->len;
    }

    res->conditionals = xrealloc(res->conditionals, sizeof(struct log_or_and_expr) * res->len);
    return res;

fail:
    free_cond_expr(res);
    return NULL;
}

struct cond_expr* parse_cond_expr_unary(struct parser_state* s, struct unary_expr* start) {
    (void)s;
    (void)start;

    // TODO:

    return NULL;
}

void free_cond_expr_children(struct cond_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        struct log_or_and_expr* item = &e->conditionals[i];
        free_log_or_expr(item->log_or);
        free_expr(item->expr);
    }
    free(e->conditionals);

    free_log_or_expr(e->last_else);
}

void free_cond_expr(struct cond_expr* e) {
    free_cond_expr_children(e);
    free(e);
}

