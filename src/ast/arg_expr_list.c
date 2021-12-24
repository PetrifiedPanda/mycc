#include "ast/arg_expr_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

struct arg_expr_list parse_arg_expr_list(struct parser_state* s) {
    size_t alloc_len = 1;
    struct arg_expr_list res = {.len = 0, .assign_exprs = xmalloc(sizeof(struct assign_expr) * alloc_len)};
    if (!parse_assign_expr_inplace(s, &res.assign_exprs[0])) {
        goto fail;
    }
    res.len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.assign_exprs, &alloc_len, sizeof(struct assign_expr));
        }

        if (!parse_assign_expr_inplace(s, &res.assign_exprs[res.len])) {
            goto fail;
        }

        ++res.len;
    }

    res.assign_exprs = xrealloc(res.assign_exprs, sizeof(struct assign_expr) * res.len);
    return res;

fail:
    free_arg_expr_list(&res);
    return (struct arg_expr_list){.assign_exprs = NULL, .len = 0};
}

void free_arg_expr_list(struct arg_expr_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_assign_expr_children(&l->assign_exprs[i]);
    }
    free(l->assign_exprs);
}

