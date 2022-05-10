#ifndef IDENTIFIER_LIST_H
#define IDENTIFIER_LIST_H

#include <stddef.h>

#include "parser/parser_state.h"

struct identifier;

struct identifier_list {
    size_t len;
    struct identifier* identifiers;
};

struct identifier_list parse_identifier_list(struct parser_state* s);

void free_identifier_list(struct identifier_list* l);

#include "ast/identifier.h"

#endif
