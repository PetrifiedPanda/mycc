#ifndef POINTER_H
#define POINTER_H

#include <stddef.h>

#include "ast/declaration/type_quals.h"

#include "parser/parser_state.h"

struct pointer {
    size_t num_indirs;
    struct type_quals* quals_after_ptr;
};

struct pointer* parse_pointer(struct parser_state* s);

void free_pointer(struct pointer* p);

#endif

