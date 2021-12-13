#ifndef TYPE_QUAL_LIST_H
#define TYPE_QUAL_LIST_H

#include <stddef.h>

struct type_qual;

struct type_qual_list {
    size_t len;
    struct type_qual* type_quals;
};

struct type_qual_list create_type_qual_list(struct type_qual* type_quals, size_t len);

void free_type_qual_list(struct type_qual_list* l);

#include "ast/type_qual.h"

#endif

