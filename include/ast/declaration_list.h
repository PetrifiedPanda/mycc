#ifndef DECLARATION_LIST_H
#define DECLARATION_LIST_H

#include <stddef.h>

typedef struct Declaration Declaration;

typedef struct DeclarationList {
    size_t len;
    Declaration* decls;
} DeclarationList;

DeclarationList* create_declaration_list(Declaration* decls, size_t len);

void free_declaration_list(DeclarationList* l);

#include "ast/declaration.h"

#endif
