#ifndef SHIFT_EXPR_H
#define SHIFT_EXPR_H

#include <stddef.h>

#include "frontend/token_kind.h"

#include "frontend/parser/parser_state.h"

struct add_expr;

enum shift_expr_op {
    SHIFT_EXPR_LEFT,
    SHIFT_EXPR_RIGHT,
};

struct add_expr_and_op {
    enum shift_expr_op op;
    struct add_expr* rhs;
};

struct shift_expr {
    struct add_expr* lhs;
    size_t len;
    struct add_expr_and_op* shift_chain;
};

struct shift_expr* parse_shift_expr(struct parser_state* s);

struct cast_expr;

struct shift_expr* parse_shift_expr_cast(struct parser_state* s,
                                         struct cast_expr* start);

void free_shift_expr(struct shift_expr* e);

#include "add_expr.h"
#include "cast_expr.h"

#endif

