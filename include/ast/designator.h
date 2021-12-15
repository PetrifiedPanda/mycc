#ifndef DESIGNATOR_H
#define DESIGNATOR_H

#include <stdbool.h>

struct const_expr;
struct identifier;

struct designator {
    bool is_index;
    union {
        struct const_expr* arr_index;
        struct identifier* identifier;
    };
};

void free_designator_children(struct designator* d);

#include "ast/const_expr.h"
#include "ast/identifier.h"

#endif
