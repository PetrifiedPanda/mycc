#ifndef XOR_EXPR_H
#define XOR_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct and_expr;

struct xor_expr {
    size_t len;
    struct and_expr* and_exprs;
};

bool parse_xor_expr_inplace(struct parser_state* s, struct xor_expr* res);

struct unary_expr;

struct xor_expr* parse_xor_expr_unary(struct parser_state* s, struct unary_expr* start);

void free_xor_expr_children(struct xor_expr* e);

void free_xor_expr(struct xor_expr* e);

#include "ast/and_expr.h"

#endif

