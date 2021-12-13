#ifndef SHIFT_EXPR_H
#define SHIFT_EXPR_H

#include <stddef.h>

#include "token_type.h"

struct add_expr;

struct add_expr_and_op {
    enum token_type shift_op;
    struct add_expr* rhs;
};

struct shift_expr {
    struct add_expr* lhs;
    size_t len;
    struct add_expr_and_op* shift_chain;
};

struct shift_expr* create_shift_expr(struct add_expr* lhs, struct add_expr_and_op* shift_chain, size_t len);

void free_shift_expr(struct shift_expr* e);

#include "ast/add_expr.h"

#endif

