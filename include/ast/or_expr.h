#ifndef OR_EXPR_H
#define OR_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct xor_expr;

struct or_expr {
    size_t len;
    struct xor_expr* xor_exprs;
};

bool parse_or_expr_inplace(struct parser_state* s, struct or_expr* res);

struct cast_expr;

struct or_expr* parse_or_expr_cast(struct parser_state* s, struct cast_expr* start);

void free_or_expr_children(struct or_expr* e);

void free_or_expr(struct or_expr* e);

#include "ast/xor_expr.h"
#include "ast/cast_expr.h"

#endif

