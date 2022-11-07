#ifndef STRUCT_DECLARATION_LIST_H
#define STRUCT_DECLARATION_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct struct_declaration;

struct struct_declaration_list {
    size_t len;
    struct struct_declaration* decls;
};

bool parse_struct_declaration_list(struct parser_state* s,
                                   struct struct_declaration_list* res);

void free_struct_declaration_list(struct struct_declaration_list* l);

#include "struct_declaration.h"

#endif

