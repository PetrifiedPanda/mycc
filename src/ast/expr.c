#include "ast/expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_expr_inplace(struct parser_state* s, struct expr* res) {
    assert(res);

    res->assign_exprs = xmalloc(sizeof(struct assign_expr));

    if (!parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        res->len = 0;
        goto fail;
    }

    size_t alloc_len = res->len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (alloc_len == res->len) {
            grow_alloc((void**)&res->assign_exprs, &alloc_len, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->assign_exprs = xrealloc(res->assign_exprs, sizeof(struct assign_expr) * res->len);
    }

    return true;
fail:
    free_expr_children(res);
    return false;
}

struct expr* parse_expr(struct parser_state* s) {
    struct expr* res = xmalloc(sizeof(struct expr));
    if (!parse_expr_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_expr_children(struct expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_assign_expr_children(&e->assign_exprs[i]);
    }
    free(e->assign_exprs);
}

void free_expr(struct expr* e) {
    free_expr_children(e);
    free(e);  
}

