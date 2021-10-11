#ifndef SPEC_QUAL_LIST_H
#define SPEC_QUAL_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct TypeSpec TypeSpec;
typedef struct TypeQual TypeQual;

typedef struct {
    bool is_type_spec;
    union {
        TypeSpec* type_spec;
        TypeQual* type_qual;
    };
} TypeSpecOrQual;

typedef struct SpecQualList {
    size_t len;
    TypeSpecOrQual* specs_or_quals;
} SpecQualList;

void free_spec_qual_list(SpecQualList* l);

#include "ast/type_spec.h"
#include "ast/type_qual.h"

#endif