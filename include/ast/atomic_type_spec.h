#ifndef ATOMIC_TYPE_SPEC_H
#define ATOMIC_TYPE_SPEC_H

struct type_name;

struct atomic_type_spec {
    struct type_name* type_name;
};

void free_atomic_type_spec(struct atomic_type_spec* s);

#include "ast/type_name.h"

#endif