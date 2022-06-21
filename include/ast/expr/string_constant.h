#ifndef STRING_CONSTANT_H
#define STRING_CONSTANT_H

#include <stdbool.h>

#include "ast/string_literal.h"

struct string_constant {
    bool is_func;
    struct string_literal lit;
};

struct string_constant create_string_constant(char* spelling);

struct string_constant create_func_name(void);

void free_string_constant(struct string_constant* c);

#endif
