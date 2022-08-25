#ifndef EQ_EXPR_H
#define EQ_EXPR_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct rel_expr;

enum eq_expr_op {
    EQ_EXPR_EQ,
    EQ_EXPR_NE,
};

struct rel_expr_and_op {
    enum eq_expr_op op;
    struct rel_expr* rhs;
};

struct eq_expr {
    struct rel_expr* lhs;
    size_t len;
    struct rel_expr_and_op* eq_chain;
};

bool parse_eq_expr_inplace(struct parser_state* s, struct eq_expr* res);

struct cast_expr;

struct eq_expr* parse_eq_expr_cast(struct parser_state* s,
                                   struct cast_expr* start);

void free_eq_expr_children(struct eq_expr* e);

#include "rel_expr.h"
#include "cast_expr.h"

#endif

