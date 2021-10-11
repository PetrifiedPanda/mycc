#ifndef POINTER_H
#define POINTER_H

#include <stddef.h>

typedef struct TypeQualList TypeQualList;

typedef struct Pointer {
    size_t num_indirs;
    TypeQualList** quals_after_ptr;
} Pointer;

Pointer* create_pointer(TypeQualList** quals_after_ptr, size_t num_indirs);

void free_pointer(Pointer* p);

#include "ast/type_qual_list.h"

#endif