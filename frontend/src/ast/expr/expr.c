#include "frontend/ast/expr/expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_expr_inplace(struct parser_state* s, struct expr* res) {
    assert(res);

    res->assign_exprs = mycc_alloc(sizeof *res->assign_exprs);

    if (!parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        mycc_free(res->assign_exprs);
        return false;
    }
    res->len = 1;
    size_t alloc_len = res->len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (alloc_len == res->len) {
            mycc_grow_alloc((void**)&res->assign_exprs,
                       &alloc_len,
                       sizeof *res->assign_exprs);
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            free_expr_children(res);
            return false;
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->assign_exprs = mycc_realloc(res->assign_exprs,
                                     sizeof *res->assign_exprs * res->len);
    }

    return true;
}

struct expr* parse_expr(struct parser_state* s) {
    struct expr* res = mycc_alloc(sizeof *res);
    if (!parse_expr_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void free_expr_children(struct expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_assign_expr_children(&e->assign_exprs[i]);
    }
    mycc_free(e->assign_exprs);
}

void free_expr(struct expr* e) {
    free_expr_children(e);
    mycc_free(e);
}
