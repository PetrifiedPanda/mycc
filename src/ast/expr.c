#include "ast/expr.h"

#include <stdlib.h>
#include <assert.h>

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

