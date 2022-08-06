#ifndef GENERIC_ASOC_LIST_H
#define GENERIC_ASOC_LIST_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct generic_assoc;

struct generic_assoc_list {
    struct ast_node_info info;
    size_t len;
    struct generic_assoc* assocs;
};

struct generic_assoc_list parse_generic_assoc_list(struct parser_state* s);

void free_generic_assoc_list(struct generic_assoc_list* l);

#include "generic_assoc.h"

#endif
