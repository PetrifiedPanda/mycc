#ifndef REL_EXPR_H
#define REL_EXPR_H

#include <stddef.h>

#include "token_type.h"

#include "parser/parser_state.h"

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

struct rel_expr* parse_rel_expr(struct parser_state* s);

struct unary_expr;

struct rel_expr* parse_rel_expr_unary(struct parser_state* s, struct unary_expr* start);

void free_rel_expr_children(struct rel_expr* e);
void free_rel_expr(struct rel_expr* e);

#include "ast/shift_expr.h"

#endif

