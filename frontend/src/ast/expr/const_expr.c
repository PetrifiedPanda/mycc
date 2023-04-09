#include "frontend/ast/expr/const_expr.h"

#include "util/mem.h"

struct const_expr* parse_const_expr(struct parser_state* s) {
    struct const_expr* res = mycc_alloc(sizeof *res);
    if (!parse_cond_expr_inplace(s, &res->expr)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static void free_const_expr_children(struct const_expr* e) {
    free_cond_expr_children(&e->expr);
}

void free_const_expr(struct const_expr* e) {
    free_const_expr_children(e);
    mycc_free(e);
}

