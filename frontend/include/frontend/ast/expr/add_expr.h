#ifndef ADD_EXPR_H
#define ADD_EXPR_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct mul_expr;

enum add_expr_op {
    ADD_EXPR_ADD,
    ADD_EXPR_SUB,
};

struct mul_expr_and_op {
    enum add_expr_op op;
    struct mul_expr* rhs;
};

struct add_expr {
    struct mul_expr* lhs;
    size_t len;
    struct mul_expr_and_op* add_chain;
};

struct add_expr* parse_add_expr(struct parser_state* s);

struct cast_expr;

struct add_expr* parse_add_expr_cast(struct parser_state* s,
                                     struct cast_expr* start);

void free_add_expr(struct add_expr* e);

#include "mul_expr.h"
#include "cast_expr.h"

#endif

