#ifndef CONST_EXPR_H
#define CONST_EXPR_H

#include "ast/cond_expr.h"

struct const_expr {
    struct cond_expr expr;
};

struct const_expr* create_const_expr(struct cond_expr expr);

void free_const_expr(struct const_expr* e);

#endif

