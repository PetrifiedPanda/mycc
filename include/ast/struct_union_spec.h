#ifndef STRUCT_UNION_SPEC_H
#define STRUCT_UNION_SPEC_H

#include <stdbool.h>

#include "ast/struct_declaration_list.h"

struct identifier;

struct struct_union_spec {
    bool is_struct;
    struct identifier* identifier;
    struct struct_declaration_list decl_list;
};

struct struct_union_spec* create_struct_union_spec(bool is_struct, struct identifier* identifier, struct struct_declaration_list decl_list);

void free_struct_union_spec(struct struct_union_spec* s);

#include "ast/identifier.h"

#endif

