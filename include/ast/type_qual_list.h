#ifndef TYPE_QUAL_LIST_H
#define TYPE_QUAL_LIST_H

#include <stddef.h>

#include "ast/type_qual.h"

typedef struct TypeQualList {
    size_t len;
    TypeQual* type_quals;
} TypeQualList;

TypeQualList create_type_qual_list(TypeQual* type_quals, size_t len);

void free_type_qual_list(TypeQualList* l);

#endif
