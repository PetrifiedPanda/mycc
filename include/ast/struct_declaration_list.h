#ifndef STRUCT_DECLARATION_LIST_H
#define STRUCT_DECLARATION_LIST_H

#include <stddef.h>

struct struct_declaration;

struct struct_declaration_list {
    size_t len;
    struct struct_declaration* decls;
};

struct struct_declaration_list create_struct_declaration_list(struct struct_declaration* decls, size_t len);

void free_struct_declaration_list(struct struct_declaration_list* l);

#include "ast/struct_declaration.h"

#endif
