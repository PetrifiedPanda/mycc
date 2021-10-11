#ifndef STRUCT_UNION_SPEC_H
#define STRUCT_UNION_SPEC_H

#include <stdbool.h>

typedef struct StructDeclarationList StructDeclarationList;

typedef struct StructUnionSpec {
    bool is_struct;
    char* identifier;
    StructDeclarationList* decl_list;
} StructUnionSpec;

StructUnionSpec* create_struct_union_spec(bool is_struct, char* identifier, StructDeclarationList* decl_list);

void free_struct_union_spec(StructUnionSpec* s);

#include "ast/struct_declaration_list.h"

#endif