#ifndef IDENTIFIER_LIST_H
#define IDENTIFIER_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct identifier;

struct identifier_list {
    size_t len;
    struct identifier* identifiers;
};

bool parse_identifier_list(struct parser_state* s, struct identifier_list* res);

void free_identifier_list(struct identifier_list* l);

#include "frontend/ast/identifier.h"

#endif
