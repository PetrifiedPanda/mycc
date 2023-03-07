#ifndef ENUM_SPEC_H
#define ENUM_SPEC_H

#include <stddef.h>

#include "frontend/ast/ast_node_info.h"

#include "frontend/parser/parser_state.h"

struct identifier;
struct const_expr;

struct enumerator {
    struct identifier* identifier;
    struct const_expr* enum_val;
};

struct enum_list {
    size_t len;
    struct enumerator* enums;
};

struct identifier;

struct enum_spec {
    struct ast_node_info info;
    struct identifier* identifier;
    struct enum_list enum_list;
};

struct enum_spec* parse_enum_spec(struct parser_state* s);

void free_enum_spec(struct enum_spec* s);

void free_enum_list(struct enum_list* l);

void free_enumerator_children(struct enumerator* e);

#include "frontend/ast/identifier.h"

#include "frontend/ast/expr/const_expr.h"

#endif

