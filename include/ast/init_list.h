#ifndef INIT_LIST_H
#define INIT_LIST_H

// TODO: update with new grammar

#include <stddef.h>

struct initializer;

struct init_list {
    size_t len;
    struct initializer* inits;
};

struct init_list create_init_list(struct initializer* inits, size_t len);

void free_init_list_children(struct init_list* l);

#include "ast/initializer.h"

#endif

