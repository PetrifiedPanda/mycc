#ifndef INIT_LIST_H
#define INIT_LIST_H

#include <stddef.h>

struct initializer;
struct designation;

struct designation_init {
    struct designation* designation;
    struct initializer* init;
};

struct init_list {
    size_t len;
    struct designation_init* inits;
};

struct init_list create_init_list(struct designation_init* inits, size_t len);

void free_init_list_children(struct init_list* l);

#include "ast/initializer.h"
#include "ast/designation.h"

#endif

