#ifndef DESIGNATOR_LIST_H
#define DESIGNATOR_LIST_H

#include <stddef.h>

#include "parser/parser_state.h"

struct designator;

struct designator_list {
    size_t len;
    struct designator* designators;
};

struct designator_list parse_designator_list(struct parser_state* s);

void free_designator_list(struct designator_list* l);

#include "ast/initializer/designator.h"

#endif

