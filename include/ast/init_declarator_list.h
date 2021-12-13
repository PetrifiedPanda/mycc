#ifndef INIT_DECLARATOR_LIST_H
#define INIT_DECLARATOR_LIST_H

#include <stddef.h>

struct init_declarator;

struct init_declarator_list {
    size_t len;
    struct init_declarator* decls;
};

struct init_declarator_list create_init_declarator_list(struct init_declarator* decls, size_t len);

void free_init_declarator_list(struct init_declarator_list* l);

#include "ast/init_declarator.h"

#endif

