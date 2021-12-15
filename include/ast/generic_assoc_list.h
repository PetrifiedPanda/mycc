#ifndef GENERIC_ASOC_LIST_H
#define GENERIC_ASOC_LIST_H

#include <stddef.h>

struct generic_assoc;

struct generic_assoc_list {
    size_t len;
    struct generic_assoc* assocs;
};

void free_generic_assoc_list(struct generic_assoc_list* l);

#include "ast/generic_assoc.h"

#endif
