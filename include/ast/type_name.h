#ifndef TYPE_NAME_H
#define TYPE_NAME_H

#include "ast/spec_qual_list.h"

typedef struct AbstractDeclarator AbstractDeclarator;

typedef struct TypeName {
    SpecQualList spec_qual_list;
    AbstractDeclarator* abstract_decl;
} TypeName;

TypeName* create_type_name(SpecQualList spec_qual_list, AbstractDeclarator* abstract_decl);

void free_type_name_children(TypeName* n);
void free_type_name(TypeName* n);

#include "ast/abstract_declarator.h"

#endif
