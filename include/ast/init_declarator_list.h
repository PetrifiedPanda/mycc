#ifndef INIT_DECLARATOR_LIST_H
#define INIT_DECLARATOR_LIST_H

#include <stddef.h>

#include "parser/parser_state.h"

struct init_declarator;

struct init_declarator_list {
    size_t len;
    struct init_declarator* decls;
};

struct init_declarator_list parse_init_declarator_list(struct parser_state* s);

void free_init_declarator_list(struct init_declarator_list* l);

#include "ast/init_declarator.h"

#endif

