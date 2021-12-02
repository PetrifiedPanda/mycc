#ifndef STRUCT_UNION_SPEC_H
#define STRUCT_UNION_SPEC_H

#include <stdbool.h>

#include "ast/struct_declaration_list.h"

typedef struct Identifier Identifier;

typedef struct StructUnionSpec {
    bool is_struct;
    Identifier* identifier;
    StructDeclarationList decl_list;
} StructUnionSpec;

StructUnionSpec* create_struct_union_spec(bool is_struct, Identifier* identifier, StructDeclarationList decl_list);

void free_struct_union_spec(StructUnionSpec* s);

#include "ast/identifier.h"

#endif

