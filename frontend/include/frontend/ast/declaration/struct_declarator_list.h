#ifndef STRUCT_DECLARATOR_LIST_H
#define STRUCT_DECLARATOR_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct struct_declarator;

struct struct_declarator_list {
    size_t len;
    struct struct_declarator* decls;
};

bool parse_struct_declarator_list(struct parser_state* s,
                                  struct struct_declarator_list* res);

void free_struct_declarator_list(struct struct_declarator_list* l);

#include "struct_declarator.h"

#endif

