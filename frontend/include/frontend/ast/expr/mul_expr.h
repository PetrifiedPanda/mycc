#ifndef MUL_EXPR_H
#define MUL_EXPR_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct cast_expr;

enum mul_expr_op {
    MUL_EXPR_MUL,
    MUL_EXPR_DIV,
    MUL_EXPR_MOD,
};

struct cast_expr_and_op {
    enum mul_expr_op op;
    struct cast_expr* rhs;
};

struct mul_expr {
    struct cast_expr* lhs;
    size_t len;
    struct cast_expr_and_op* mul_chain;
};

struct mul_expr* parse_mul_expr(struct parser_state* s);

struct cast_expr;

struct mul_expr* parse_mul_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_mul_expr(struct mul_expr* e);

#include "cast_expr.h"

#endif

