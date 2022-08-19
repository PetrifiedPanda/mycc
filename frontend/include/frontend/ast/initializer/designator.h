#ifndef DESIGNATOR_H
#define DESIGNATOR_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct const_expr;
struct identifier;

struct designator {
    struct ast_node_info info;
    bool is_index;
    union {
        struct const_expr* arr_index;
        struct identifier* identifier;
    };
};

bool parse_designator_inplace(struct parser_state* s, struct designator* res);

void free_designator_children(struct designator* d);

#include "frontend/ast/expr/const_expr.h"
#include "frontend/ast/identifier.h"

#endif

