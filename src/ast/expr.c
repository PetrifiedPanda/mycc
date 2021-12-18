#include "ast/expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

struct expr* parse_expr(struct parser_state* s) {
    size_t num_elems = 1;
    struct expr* res = xmalloc(sizeof(struct expr));
    res->assign_exprs = xmalloc(num_elems * sizeof(struct assign_expr));

    if (parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        res->len = 0;
        goto fail;
    }

    res->len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (num_elems == res->len) {
            grow_alloc((void**)&res->assign_exprs, &num_elems, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    if (num_elems != res->len) {
        res->assign_exprs = xrealloc(res->assign_exprs, sizeof(struct assign_expr) * res->len);
    }

    return res;
    fail:
    free_expr(res);
    return NULL;
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

