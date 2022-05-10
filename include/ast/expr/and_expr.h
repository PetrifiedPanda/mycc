#ifndef AND_EXPR_H
#define AND_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct eq_expr;

struct and_expr {
    struct eq_expr* eq_exprs;
    size_t len;
};

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res);

struct cast_expr;

struct and_expr* parse_and_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_and_expr_children(struct and_expr* e);

#include "ast/expr/eq_expr.h"
#include "ast/expr/cast_expr.h"

#endif
