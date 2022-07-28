#ifndef DECLARATION_LIST_H
#define DECLARATION_LIST_H

#include <stddef.h>

#include "parser/parser_state.h"

struct declaration;

struct declaration_list {
    size_t len;
    struct declaration* decls;
};

struct declaration_list parse_declaration_list(struct parser_state* s);

void free_declaration_list(struct declaration_list* l);

#include "ast/declaration/declaration.h"

#endif

