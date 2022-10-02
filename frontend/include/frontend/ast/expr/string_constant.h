#ifndef STRING_CONSTANT_H
#define STRING_CONSTANT_H

#include <stdbool.h>

#include "frontend/ast/string_literal.h"

struct string_constant {
    bool is_func;
    union {
        struct string_literal lit;
        struct ast_node_info info;
    };
};

struct string_constant create_string_constant(const struct str* spelling, struct source_loc loc);

struct string_constant create_func_name(struct source_loc loc);

void free_string_constant(struct string_constant* c);

#endif

