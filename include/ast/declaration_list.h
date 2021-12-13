#ifndef DECLARATION_LIST_H
#define DECLARATION_LIST_H

#include <stddef.h>

struct declaration;

struct declaration_list {
    size_t len;
    struct declaration* decls;
};

struct declaration_list* create_declaration_list(struct declaration* decls, size_t len);

void free_declaration_list(struct declaration_list* l);

#include "ast/declaration.h"

#endif

