#ifndef CAST_EXPR_H
#define CAST_EXPR_H

#include <stdlib.h>

struct unary_expr;
struct type_name;

struct cast_expr {
    size_t len;
    struct type_name* type_names;
    struct unary_expr* rhs;
};

struct cast_expr* create_cast_expr(struct type_name* type_names, size_t len, struct unary_expr* rhs);

void free_cast_expr(struct cast_expr* e);

#include "ast/unary_expr.h"
#include "ast/type_name.h"

#endif

