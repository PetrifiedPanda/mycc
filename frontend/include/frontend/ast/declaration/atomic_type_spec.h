#ifndef ATOMIC_TYPE_SPEC_H
#define ATOMIC_TYPE_SPEC_H

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct type_name;

struct atomic_type_spec {
    struct ast_node_info info;
    struct type_name* type_name;
};

struct atomic_type_spec* parse_atomic_type_spec(struct parser_state* s);

void free_atomic_type_spec(struct atomic_type_spec* s);

#include "frontend/ast/type_name.h"

#endif

