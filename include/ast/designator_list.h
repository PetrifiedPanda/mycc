#ifndef DESIGNATOR_LIST_H
#define DESIGNATOR_LIST_H

#include <stddef.h>

struct designator;

struct designator_list {
    size_t len;
    struct designator* designators;
};

void free_designator_list(struct designator_list* l);

#include "ast/designator.h"

#endif
