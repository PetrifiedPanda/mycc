#ifndef INIT_DECLARATOR_LIST_H
#define INIT_DECLARATOR_LIST_H

#include <stddef.h>

typedef struct InitDeclarator InitDeclarator;

typedef struct InitDeclaratorList {
    size_t len;
    InitDeclarator* decls;
} InitDeclaratorList;

InitDeclaratorList create_init_declarator_list(InitDeclarator* decls, size_t len);

void free_init_declarator_list(InitDeclaratorList* l);

#include "ast/init_declarator.h"

#endif

