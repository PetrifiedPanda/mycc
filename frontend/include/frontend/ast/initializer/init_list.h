#ifndef INIT_LIST_H
#define INIT_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

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

bool parse_init_list(struct parser_state* s, struct init_list* res);

void free_init_list_children(struct init_list* l);

#include "initializer.h"
#include "designation.h"

#endif

