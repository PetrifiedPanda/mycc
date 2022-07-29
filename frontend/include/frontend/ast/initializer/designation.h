#ifndef DESIGNATION_H
#define DESIGNATION_H

#include "designator_list.h"

#include "frontend/parser/parser_state.h"

struct designation {
    struct designator_list designators;
};

struct designation* parse_designation(struct parser_state* s);

void free_designation(struct designation* d);

#endif
