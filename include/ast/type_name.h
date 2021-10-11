#ifndef TYPE_NAME_H
#define TYPE_NAME_H

typedef struct SpecQualList SpecQualList;
typedef struct AbstractDecl AbstractDecl;

typedef struct TypeName {
    SpecQualList* spec_qual_list;
    AbstractDecl* abstract_decl;
} TypeName;

TypeName* create_type_name(SpecQualList* spec_qual_list, AbstractDecl* abstract_decl);

void free_type_name_children(TypeName* n);
void free_type_name(TypeName* n);

#include "ast/spec_qual_list.h"
#include "ast/abstract_decl.h"

#endif