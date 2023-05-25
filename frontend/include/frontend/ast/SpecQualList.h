#ifndef SPEC_QUAL_LIST_H
#define SPEC_QUAL_LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "frontend/ast/declaration/TypeQuals.h"
#include "frontend/ast/declaration/TypeSpecs.h"

typedef struct SpecQualList {
    AstNodeInfo info;
    TypeQuals quals;
    TypeSpecs specs;
} SpecQualList;

bool parse_spec_qual_list(ParserState* s, SpecQualList* res);

void free_spec_qual_list_children(SpecQualList* l);
void free_spec_qual_list(SpecQualList* l);

#endif

