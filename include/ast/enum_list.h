#ifndef ENUM_LIST_H
#define ENUM_LIST_H

#include <stddef.h>

struct enumerator;

struct enum_list {
    size_t len;
    struct enumerator* enums;
};

struct enum_list create_enum_list(struct enumerator* enums, size_t len);

void free_enum_list(struct enum_list* l);

#include "ast/enumerator.h"

#endif

