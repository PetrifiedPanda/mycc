#ifndef STRUCT_UNION_SPEC_H
#define STRUCT_UNION_SPEC_H

#include <stdbool.h>

#include "ast/declaration/struct_declaration_list.h"

#include "parser/parser_state.h"

struct identifier;

struct struct_union_spec {
    bool is_struct;
    struct identifier* identifier;
    struct struct_declaration_list decl_list;
};

struct struct_union_spec* parse_struct_union_spec(struct parser_state* s);

void free_struct_union_spec(struct struct_union_spec* s);

struct ast_visitor;

bool visit_struct_union_spec(struct ast_visitor* visitor,
                             struct struct_union_spec* s);

#include "ast/identifier.h"

#endif

