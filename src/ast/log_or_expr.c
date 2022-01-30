#include "ast/log_or_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static void free_children(struct log_or_expr* e);

static bool parse_log_or_expr_ops(struct parser_state* s, struct log_or_expr* res) {
    assert(res);
    assert(res->len == 1);

    size_t alloc_len = res->len;
    while (s->it->type == OR_OP) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->log_ands, &alloc_len, sizeof(struct log_and_expr));
        }

        if (!parse_log_and_expr_inplace(s, &res->log_ands[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->log_ands = xrealloc(res->log_ands, sizeof(struct log_and_expr) * res->len);

    return true;
fail:
    free_children(res);
    return false;
}

struct log_or_expr* parse_log_or_expr(struct parser_state* s) {
    struct log_and_expr* and_exprs = xmalloc(sizeof(struct log_and_expr));
    if (!parse_log_and_expr_inplace(s, and_exprs)) {
        free(and_exprs);
        return NULL;
    }

    struct log_or_expr* res = xmalloc(sizeof(struct log_or_expr));
    res->log_ands = and_exprs;
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

struct log_or_expr* parse_log_or_expr_cast(struct parser_state* s, struct cast_expr* start) {
    assert(start);

    struct log_and_expr* and_exprs = parse_log_and_expr_cast(s, start);
    if (!and_exprs) {
        return NULL;
    }

    struct log_or_expr* res = xmalloc(sizeof(struct log_or_expr));
    res->log_ands = and_exprs;
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

static void free_children(struct log_or_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_log_and_expr_children(&e->log_ands[i]);
    }
    free(e->log_ands);
}

void free_log_or_expr(struct log_or_expr* e) {
    free_children(e);
    free(e);
}

