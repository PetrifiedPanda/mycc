#ifndef ATOMIC_TYPE_SPEC_H
#define ATOMIC_TYPE_SPEC_H

#include "parser/parser_state.h"

struct type_name;

struct atomic_type_spec {
    struct type_name* type_name;
};

struct atomic_type_spec* parse_atomic_type_spec(struct parser_state* s);

void free_atomic_type_spec(struct atomic_type_spec* s);

struct ast_visitor;

bool visit_atomic_type_spec(struct ast_visitor* visitor,
                            struct atomic_type_spec* s);

#include "ast/type_name.h"

#endif

