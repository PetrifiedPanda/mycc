#ifndef MUL_EXPR_H
#define MUL_EXPR_H

#include <stddef.h>

#include "token_type.h"

struct cast_expr;

struct cast_expr_and_op {
    enum token_type mul_op;
    struct cast_expr* rhs;
};

struct mul_expr {
    struct cast_expr* lhs;
    size_t len;
    struct cast_expr_and_op* mul_chain;
};

struct mul_expr* create_mul_expr(struct cast_expr* lhs, struct cast_expr_and_op* mul_chain, size_t len);

void free_mul_expr(struct mul_expr* e);

#include "ast/cast_expr.h"

#endif

