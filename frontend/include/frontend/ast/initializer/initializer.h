#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <stdbool.h>

#include "init_list.h"

#include "frontend/parser/parser_state.h"

struct assign_expr;

struct initializer {
    bool is_assign;
    union {
        struct assign_expr* assign;
        struct init_list init_list;
    };
};

struct initializer* parse_initializer(struct parser_state* s);

void free_initializer_children(struct initializer* i);
void free_initializer(struct initializer* i);

#include "frontend/ast/expr/assign_expr.h"

#endif
