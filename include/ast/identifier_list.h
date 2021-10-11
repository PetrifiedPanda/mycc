#ifndef IDENTIFIER_LIST_H
#define IDENTIFIER_LIST_H

#include <stddef.h>

typedef struct IdentifierList {
    size_t len;
    char** identifiers;
} IdentifierList;

IdentifierList* create_identifier_list(char** identifiers, size_t len);

void free_identifier_list(IdentifierList* l);

#endif