#ifndef MUL_EXPR_H
#define MUL_EXPR_H

#include <stddef.h>

#include "token_type.h"

#include "parser/parser_state.h"

struct cast_expr;

struct cast_expr_and_op {
    enum token_type mul_op;
    struct cast_expr* rhs;
};

struct mul_expr {
    struct cast_expr* lhs;
    size_t len;
    struct cast_expr_and_op* mul_chain;
};

struct mul_expr* parse_mul_expr(struct parser_state* s);

struct unary_expr;

struct mul_expr* parse_mul_expr_unary(struct parser_state* s, struct unary_expr* start);

void free_mul_expr(struct mul_expr* e);

#include "ast/cast_expr.h"

#endif

