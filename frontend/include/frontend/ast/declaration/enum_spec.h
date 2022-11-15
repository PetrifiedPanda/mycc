#ifndef ENUM_SPEC_H
#define ENUM_SPEC_H

#include "enum_list.h"

struct identifier;

struct enum_spec {
    struct ast_node_info info;
    struct identifier* identifier;
    struct enum_list enum_list;
};

struct enum_spec* parse_enum_spec(struct parser_state* s);

void free_enum_spec(struct enum_spec* s);

#include "frontend/ast/identifier.h"

#endif

