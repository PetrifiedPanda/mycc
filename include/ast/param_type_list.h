#ifndef PARAM_TYPE_LIST_H
#define PARAM_TYPE_LIST_H

#include <stdbool.h>

struct param_list;

struct param_type_list {
    bool is_variadic;
    struct param_list* param_list;
};

struct param_type_list create_param_type_list(bool is_variadic, struct param_list* param_list);

void free_param_type_list(struct param_type_list* l);

#include "ast/param_list.h"

#endif

