#include "ast/log_and_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

#include "parser/parser_util.h"

static bool parse_log_and_expr_rest(struct parser_state* s,
                                    struct log_and_expr* res) {
    assert(res);
    size_t alloc_len = res->len = 1;
    while (s->it->type == AND_OP) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->or_exprs,
                       &alloc_len,
                       sizeof(struct or_expr));
        }

        if (!parse_or_expr_inplace(s, &res->or_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->or_exprs = xrealloc(res->or_exprs, sizeof(struct or_expr) * res->len);
    return true;
fail:
    free_log_and_expr_children(res);
    return false;
}

bool parse_log_and_expr_inplace(struct parser_state* s,
                                struct log_and_expr* res) {
    assert(res);

    res->or_exprs = xmalloc(sizeof(struct or_expr));
    if (!parse_or_expr_inplace(s, res->or_exprs)) {
        free(res->or_exprs);
        return false;
    }

    if (!parse_log_and_expr_rest(s, res)) {
        return false;
    }

    return true;
}

struct log_and_expr* parse_log_and_expr_cast(struct parser_state* s,
                                             struct cast_expr* start) {
    assert(start);

    struct or_expr* or_exprs = parse_or_expr_cast(s, start);
    if (!or_exprs) {
        return NULL;
    }

    struct log_and_expr* res = xmalloc(sizeof(struct log_and_expr));
    res->or_exprs = or_exprs;

    if (!parse_log_and_expr_rest(s, res)) {
        free(res);
        return NULL;
    }

    return res;
}

void free_log_and_expr_children(struct log_and_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_or_expr_children(&e->or_exprs[i]);
    }
    free(e->or_exprs);
}

void free_log_and_expr(struct log_and_expr* e) {
    free_log_and_expr_children(e);
    free(e);
}
