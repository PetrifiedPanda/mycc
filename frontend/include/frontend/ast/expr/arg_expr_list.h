#ifndef ARG_EXPR_LST_H
#define ARG_EXPR_LST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct assign_expr;

struct arg_expr_list {
    size_t len;
    struct assign_expr* assign_exprs;
};

struct arg_expr_list parse_arg_expr_list(struct parser_state* s);

void free_arg_expr_list(struct arg_expr_list* l);

#include "assign_expr.h"

#endif

