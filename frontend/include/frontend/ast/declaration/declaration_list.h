#ifndef DECLARATION_LIST_H
#define DECLARATION_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct declaration;

struct declaration_list {
    size_t len;
    struct declaration* decls;
};

bool parse_declaration_list(struct parser_state* s,
                            struct declaration_list* res);

void free_declaration_list(struct declaration_list* l);

#include "declaration.h"

#endif

