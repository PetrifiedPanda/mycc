#include "frontend/ast/Expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/AssignExpr.h"

bool parse_expr_inplace(ParserState* s, Expr* res) {
    assert(res);

    res->assign_exprs = mycc_alloc(sizeof *res->assign_exprs);

    if (!parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        mycc_free(res->assign_exprs);
        return false;
    }
    res->len = 1;
    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_COMMA) {
        ParserState_accept_it(s);
        if (alloc_len == res->len) {
            mycc_grow_alloc((void**)&res->assign_exprs,
                            &alloc_len,
                            sizeof *res->assign_exprs);
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            Expr_free_children(res);
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

void Expr_free_children(Expr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        AssignExpr_free_children(&e->assign_exprs[i]);
    }
    mycc_free(e->assign_exprs);
}

