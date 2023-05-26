#ifndef TYPE_NAME_H
#define TYPE_NAME_H

typedef struct SpecQualList SpecQualList;

#include "frontend/parser/ParserState.h"

#include "AstNodeInfo.h"

typedef struct AbsDeclarator AbsDeclarator;

typedef struct TypeName {
    SpecQualList* spec_qual_list;
    AbsDeclarator* abstract_decl;
} TypeName;

bool parse_type_name_inplace(ParserState* s, TypeName* res);
TypeName* parse_type_name(ParserState* s);

void TypeName_free_children(TypeName* n);
void TypeName_free(TypeName* n);

#include "SpecQualList.h"
#include "frontend/ast/declaration/AbsDeclarator.h"

#endif

