#ifndef POINTER_H
#define POINTER_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct type_quals;

struct pointer {
    struct ast_node_info info;
    size_t num_indirs;
    struct type_quals* quals_after_ptr;
};

struct pointer* parse_pointer(struct parser_state* s);

void free_pointer(struct pointer* p);

#include "type_quals.h"

#endif

