#ifndef STRUCT_DECLARATOR_LIST_H
#define STRUCT_DECLARATOR_LIST_H

#include <stddef.h>

struct struct_declarator;

struct struct_declarator_list {
    size_t len;
    struct struct_declarator* decls;
};

struct struct_declarator_list create_struct_declarator_list(struct struct_declarator* decls, size_t len);

void free_struct_declarator_list(struct struct_declarator_list* l);

#include "ast/struct_declarator.h"

#endif
