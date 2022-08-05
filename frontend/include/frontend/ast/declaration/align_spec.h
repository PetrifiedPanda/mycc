#ifndef ALIGN_SPEC_H
#define ALIGN_SPEC_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

struct type_name;
struct const_expr;

struct align_spec {
    bool is_type_name;
    union {
        struct type_name* type_name;
        struct const_expr* const_expr;
    };
};

bool parse_align_spec_inplace(struct parser_state* s, struct align_spec* res);

void free_align_spec_children(struct align_spec* s);

#include "frontend/ast/type_name.h"

#include "frontend/ast/expr/const_expr.h"

#endif
