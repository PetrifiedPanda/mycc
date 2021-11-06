#include "ast/arg_expr_list.h"

#include <stdlib.h>
#include <assert.h>

ArgExprList create_arg_expr_lst(AssignExpr* assign_exprs, size_t len) {
    if (len > 0) {
        assert(assign_exprs);
    } else {
        assert(assign_exprs == NULL);
    }

    ArgExprList res;

    res.assign_exprs = assign_exprs;
    res.len = len;
    return res;
}

void free_arg_expr_list(ArgExprList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_assign_expr_children(&l->assign_exprs[i]);
    }
    free(l->assign_exprs);
}

