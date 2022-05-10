#ifndef GENERIC_ASOC_H
#define GENERIC_ASOC_H

#include <stdbool.h>

#include "parser/parser_state.h"

struct type_name;
struct assign_expr;

struct generic_assoc {
    struct type_name* type_name; // if NULL this is the default case
    struct assign_expr* assign;
};

bool parse_generic_assoc_inplace(struct parser_state* s,
                                 struct generic_assoc* res);

void free_generic_assoc_children(struct generic_assoc* a);

#include "ast/type_name.h"

#include "ast/expr/assign_expr.h"

#endif
