#ifndef AND_EXPR_H
#define AND_EXPR_H

#include <stddef.h>

struct eq_expr;

struct and_expr {
    struct eq_expr* eq_exprs;
    size_t len;
};

struct and_expr* create_and_expr(struct eq_expr* eq_exprs, size_t len);

void free_and_expr_children(struct and_expr* e);

void free_and_expr(struct and_expr* e);

#include "ast/eq_expr.h"

#endif

