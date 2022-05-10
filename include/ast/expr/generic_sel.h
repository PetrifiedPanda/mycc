#ifndef GENERIC_SEL_H
#define GENERIC_SEL_H

#include "ast/expr/generic_assoc_list.h"

struct assign_expr;

struct generic_sel {
    struct assign_expr* assign;
    struct generic_assoc_list assocs;
};

struct generic_sel* parse_generic_sel(struct parser_state* s);

void free_generic_sel(struct generic_sel* s);

#include "ast/expr/assign_expr.h"

#endif
