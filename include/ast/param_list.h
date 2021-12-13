#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include <stddef.h>

struct param_declaration;

struct param_list {
    size_t len;
    struct param_declaration* decls;
};

struct param_list* create_param_list(struct param_declaration* decls, size_t len);

void free_param_list(struct param_list* l);

#include "ast/param_declaration.h"

#endif

