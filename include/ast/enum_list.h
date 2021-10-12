#ifndef ENUM_LIST_H
#define ENUM_LIST_H

#include <stddef.h>

typedef struct Enumerator Enumerator;

typedef struct EnumList {
    size_t len;
    Enumerator* enums;
} EnumList;

EnumList* create_enum_list(Enumerator* enums, size_t len);

void free_enum_list(EnumList* l);

#include "ast/enumerator.h"

#endif
