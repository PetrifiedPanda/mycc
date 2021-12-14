#ifndef IDENTIFIER_LIST_H
#define IDENTIFIER_LIST_H

#include <stddef.h>

struct identifier;

struct identifier_list {
    size_t len;
    struct identifier* identifiers;
};

struct identifier_list create_identifier_list(struct identifier* identifiers, size_t len);

void free_identifier_list(struct identifier_list* l);

#include "ast/identifier.h"

#endif

