#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct param_declaration;

struct param_list {
    size_t len;
    struct param_declaration* decls;
};

struct param_list* parse_param_list(struct parser_state* s);

void free_param_list(struct param_list* l);

#include "param_declaration.h"

#endif
