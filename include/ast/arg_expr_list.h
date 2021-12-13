#ifndef ARG_EXPR_LST_H
#define ARG_EXPR_LST_H

#include <stddef.h>

struct assign_expr;

struct arg_expr_list {
    size_t len;
    struct assign_expr* assign_exprs;
};

struct arg_expr_list create_arg_expr_list(struct assign_expr* assign_exprs, size_t len);

void free_arg_expr_list(struct arg_expr_list* l);

#include "ast/assign_expr.h"

#endif

