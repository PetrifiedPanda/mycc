#include "frontend/ast/expr/arg_expr_list.h"

#include <stdlib.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_arg_expr_list(struct parser_state* s, struct arg_expr_list* res) {
    res->len = 1;
    res->assign_exprs = xmalloc(sizeof *res->assign_exprs);
    if (!parse_assign_expr_inplace(s, &res->assign_exprs[0])) {
        free(res->assign_exprs);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->assign_exprs,
                       &alloc_len,
                       sizeof *res->assign_exprs);
        }

        if (!parse_assign_expr_inplace(s, &res->assign_exprs[res->len])) {
            free_arg_expr_list(res);
            return false;
        }

        ++res->len;
    }

    res->assign_exprs = xrealloc(res->assign_exprs,
                                 sizeof *res->assign_exprs * res->len);
    return res;
}

void free_arg_expr_list(struct arg_expr_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_assign_expr_children(&l->assign_exprs[i]);
    }
    free(l->assign_exprs);
}
