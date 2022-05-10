#ifndef DESIGNATION_H
#define DESIGNATION_H

#include "ast/initializer/designator_list.h"

#include "parser/parser_state.h"

struct designation {
    struct designator_list designators;
};

struct designation* parse_designation(struct parser_state* s);

void free_designation(struct designation* d);

#endif
