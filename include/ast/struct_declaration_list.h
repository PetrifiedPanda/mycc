#ifndef STRUCT_DECLARATION_LIST_H
#define STRUCT_DECLARATION_LIST_H

#include <stddef.h>

typedef struct StructDeclaration StructDeclaration;

typedef struct StructDeclarationList {
    size_t len;
    StructDeclaration* decls;
} StructDeclarationList;

StructDeclarationList create_struct_declaration_list(StructDeclaration* decls, size_t len);

void free_struct_declaration_list(StructDeclarationList* l);

#include "ast/struct_declaration.h"

#endif
