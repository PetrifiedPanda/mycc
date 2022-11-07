#ifndef DESIGNATOR_LIST_H
#define DESIGNATOR_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct designator;

struct designator_list {
    size_t len;
    struct designator* designators;
};

bool parse_designator_list(struct parser_state* s, struct designator_list* res);

void free_designator_list(struct designator_list* l);

#include "designator.h"

#endif

