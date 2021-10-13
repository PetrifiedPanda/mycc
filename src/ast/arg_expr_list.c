#include "ast/arg_expr_list.h"

#include <stdlib.h>
#include <assert.h>

ArgExprLst* create_arg_expr_lst(AssignExpr* assign_exprs, size_t len) {
    assert(assign_exprs);
    assert(len > 0);
    ArgExprLst* res = malloc(sizeof(ArgExprLst));
    if (res) {
        res->assign_exprs = assign_exprs;
        res->len = len;
    }
    return res;
}

static void free_children(ArgExprList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_assign_expr_children(&l->assign_exprs[i]);
    }
    free(l->assign_exprs);
}

void free_arg_expr_list(ArgExprList* l) {
    free_children(l);
    free(l);
}
