#ifndef ENUM_LIST_H
#define ENUM_LIST_H

#include <stddef.h>

#include "parser/parser_state.h"

struct enumerator;

struct enum_list {
    size_t len;
    struct enumerator* enums;
};

struct enum_list parse_enum_list(struct parser_state* s);

void free_enum_list(struct enum_list* l);

#include "ast/enumerator.h"

#endif
