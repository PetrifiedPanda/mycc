#ifndef REL_EXPR_H
#define REL_EXPR_H

#include "token_type.h"

#include <stddef.h>

struct shift_expr;

struct shift_expr_and_op {
    enum token_type rel_op;
    struct shift_expr* rhs;
};

struct rel_expr {
    struct shift_expr* lhs;
    size_t len;
    struct shift_expr_and_op* rel_chain;
};

struct rel_expr* create_rel_expr(struct shift_expr* lhs, struct shift_expr_and_op* rel_chain, size_t len);

void free_rel_expr_children(struct rel_expr* e);

void free_rel_expr(struct rel_expr* e);

#include "ast/shift_expr.h"

#endif

