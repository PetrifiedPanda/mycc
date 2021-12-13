#include "ast/arg_expr_list.h"

#include <stdlib.h>
#include <assert.h>

struct arg_expr_list create_arg_expr_lst(struct assign_expr* assign_exprs, size_t len) {
    if (len > 0) {
        assert(assign_exprs);
    } else {
        assert(assign_exprs == NULL);
    }

    struct arg_expr_list res;

    res.assign_exprs = assign_exprs;
    res.len = len;
    return res;
}

void free_arg_expr_list(struct arg_expr_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_assign_expr_children(&l->assign_exprs[i]);
    }
    free(l->assign_exprs);
}

