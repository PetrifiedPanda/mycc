#ifndef PARAM_TYPE_LIST_H
#define PARAM_TYPE_LIST_H

#include <stdbool.h>

#include "parser/parser_state.h"

struct param_list;

struct param_type_list {
    bool is_variadic;
    struct param_list* param_list;
};

struct param_type_list parse_param_type_list(struct parser_state* s);

void free_param_type_list(struct param_type_list* l);

#include "ast/param_list.h"

#endif
