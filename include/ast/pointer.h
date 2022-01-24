#ifndef POINTER_H
#define POINTER_H

#include <stddef.h>

#include "parser/parser_state.h"

struct type_qual_list;

struct pointer {
    size_t num_indirs;
    struct type_qual_list* quals_after_ptr;
};

struct pointer* parse_pointer(struct parser_state* s);

void free_pointer(struct pointer* p);

#include "ast/type_qual_list.h"

#endif

