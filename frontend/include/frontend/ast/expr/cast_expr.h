#ifndef CAST_EXPR_H
#define CAST_EXPR_H

#include <stdlib.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct unary_expr;
struct type_name;

struct cast_expr {
    struct ast_node_info info;
    size_t len;
    struct type_name* type_names;
    struct unary_expr* rhs;
};

struct cast_expr* parse_cast_expr(struct parser_state* s);
struct cast_expr* parse_cast_expr_type_name(struct parser_state* s,
                                            struct type_name* type_name,
                                            struct source_loc start_bracket_loc);

struct cast_expr* create_cast_expr_unary(struct unary_expr* start);

void free_cast_expr(struct cast_expr* e);

#include "unary_expr.h"
#include "frontend/ast/type_name.h"

#endif

