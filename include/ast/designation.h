#ifndef DESIGNATION_H
#define DESIGNATION_H

#include "ast/designator_list.h"

struct designation {
    struct designator_list designators;
};

void free_designation(struct designation* d);

#endif
