#ifndef IDENTIFIER_LIST_H
#define IDENTIFIER_LIST_H

#include <stddef.h>

typedef struct Identifier Identifier;

typedef struct IdentifierList {
    size_t len;
    Identifier* identifiers;
} IdentifierList;

IdentifierList* create_identifier_list(Identifier* identifiers, size_t len);

void free_identifier_list(IdentifierList* l);

#include "ast/identifier.h"

#endif

