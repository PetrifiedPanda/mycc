#ifndef STRUCT_DECLARATOR_LIST_H
#define STRUCT_DECLARATOR_LIST_H

#include <stddef.h>

typedef struct StructDeclarator StructDeclarator;

typedef struct StructDeclaratorList {
    size_t len;
    StructDeclarator* decls;
} StructDeclaratorList;

StructDeclaratorList create_struct_declarator_list(StructDeclarator* decls, size_t len);

void free_struct_declarator_list(StructDeclaratorList* l);

#include "ast/struct_declarator.h"

#endif

