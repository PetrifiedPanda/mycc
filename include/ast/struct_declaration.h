#ifndef STRUCT_DECLARATION_H
#define STRUCT_DECLARATION_H

typedef struct SpecQualList SpecQualList;
typedef struct StructDeclaratorList StructDeclaratorList;

typedef struct StructDeclaration {
    SpecQualList* spec_qual_list;
    StructDeclaratorList* decls;
} StructDeclaration;

StructDeclaration* create_struct_declaration(SpecQualList* spec_qual_list, StructDeclaratorList* decls);

void free_struct_declaration_children(StructDeclaration* d);
void free_struct_declaration(StructDeclaration* d);

#include "ast/spec_qual_list.h"
#include "ast/struct_declarator_list.h"

#endif