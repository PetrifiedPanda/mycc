#ifndef ALIGN_SPEC_H
#define ALIGN_SPEC_H

#include <stdbool.h>

struct type_name;
struct const_expr;

struct align_spec {
    bool is_type_name;
    union {
        struct type_name* type_name;
        struct const_expr* const_expr;
    };
};

void free_align_spec(struct align_spec* s);

#include "ast/type_name.h"
#include "ast/const_expr.h"

#endif
