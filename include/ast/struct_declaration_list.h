#ifndef STRUCT_DECLARATION_LIST_H
#define STRUCT_DECLARATION_LIST_H

#include <stddef.h>

#include "parser/parser_state.h"

struct struct_declaration;

struct struct_declaration_list {
    size_t len;
    struct struct_declaration* decls;
};

struct struct_declaration_list parse_struct_declaration_list(struct parser_state* s);

void free_struct_declaration_list(struct struct_declaration_list* l);

#include "ast/struct_declaration.h"

#endif
