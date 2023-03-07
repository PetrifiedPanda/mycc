#ifndef GENERIC_SEL_H
#define GENERIC_SEL_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/ast/ast_node_info.h"

#include "frontend/parser/parser_state.h"

struct type_name;
struct assign_expr;

struct generic_assoc {
    struct ast_node_info info;
    struct type_name* type_name; // if NULL this is the default case
    struct assign_expr* assign;
};

struct generic_assoc_list {
    struct ast_node_info info;
    size_t len;
    struct generic_assoc* assocs;
};

struct assign_expr;

struct generic_sel {
    struct ast_node_info info;
    struct assign_expr* assign;
    struct generic_assoc_list assocs;
};

struct generic_sel* parse_generic_sel(struct parser_state* s);

void free_generic_sel(struct generic_sel* s);

void free_generic_assoc_list(struct generic_assoc_list* l);

void free_generic_assoc_children(struct generic_assoc* a);

#include "assign_expr.h"

#endif

