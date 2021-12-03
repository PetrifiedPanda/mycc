#include "ast/expr.h"

#include <stdlib.h>
#include <assert.h>

void free_expr_children(Expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_assign_expr_children(&e->assign_exprs[i]);
    }
    free(e->assign_exprs);
}

void free_expr(Expr* e) {
    free_expr_children(e);
    free(e);  
}

