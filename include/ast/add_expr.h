#ifndef ADDITIVE_EXPR_H
#define ADDITIVE_EXPR_H

#include <stddef.h>

#include "token_type.h"

#include "parser/parser_state.h"

struct mul_expr;

struct mul_expr_and_op {
    enum token_type add_op;
    struct mul_expr* rhs;
};

struct add_expr {
    struct mul_expr* lhs;
    size_t len;
    struct mul_expr_and_op* add_chain;
};

struct add_expr* parse_add_expr(struct parser_state* s);

struct unary_expr;

struct add_expr* parse_add_expr_unary(struct parser_state* s, struct unary_expr* start);

void free_add_expr(struct add_expr* e);

#include "ast/mul_expr.h"

#endif

