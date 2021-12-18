#ifndef CAST_EXPR_H
#define CAST_EXPR_H

#include <stdlib.h>

#include "parser/parser_state.h"

struct unary_expr;
struct type_name;

struct cast_expr {
    size_t len;
    struct type_name* type_names;
    struct unary_expr* rhs;
};

struct cast_expr* parse_cast_expr(struct parser_state* s);

void free_cast_expr(struct cast_expr* e);

#include "ast/unary_expr.h"
#include "ast/type_name.h"

#endif

