#ifndef INIT_LIST_H
#define INIT_LIST_H

// TODO: update with new grammar

#include <stddef.h>

typedef struct Initializer Initializer;

typedef struct InitList {
    size_t len;
    Initializer* inits;
} InitList;

InitList create_init_list(Initializer* inits, size_t len);

void free_init_list_children(InitList* l);

#include "ast/initializer.h"

#endif

