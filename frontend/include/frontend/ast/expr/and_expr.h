#ifndef AND_EXPR_H
#define AND_EXPR_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct eq_expr;

struct and_expr {
    size_t len;
    struct eq_expr* eq_exprs;
};

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res);

struct cast_expr;

struct and_expr* parse_and_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_and_expr_children(struct and_expr* e);

#include "eq_expr.h"
#include "cast_expr.h"

#endif

