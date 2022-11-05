#ifndef ENUM_LIST_H
#define ENUM_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct enumerator;

struct enum_list {
    size_t len;
    struct enumerator* enums;
};

bool parse_enum_list(struct parser_state* s, struct enum_list* res);

void free_enum_list(struct enum_list* l);

#include "enumerator.h"

#endif
