#include "frontend/ast/expr/ConstExpr.h"

#include "util/mem.h"

ConstExpr* parse_const_expr(ParserState* s) {
    ConstExpr* res = mycc_alloc(sizeof *res);
    if (!parse_cond_expr_inplace(s, &res->expr)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static void free_const_expr_children(ConstExpr* e) {
    CondExpr_free_children(&e->expr);
}

void ConstExpr_free(ConstExpr* e) {
    free_const_expr_children(e);
    mycc_free(e);
}

