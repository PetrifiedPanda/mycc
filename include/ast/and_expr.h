#ifndef AND_EXPR_H
#define AND_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct eq_expr;

struct and_expr {
    struct eq_expr* eq_exprs;
    size_t len;
};

bool parse_and_expr_inplace(struct parser_state* s, struct and_expr* res);

void free_and_expr_children(struct and_expr* e);
void free_and_expr(struct and_expr* e);

#include "ast/eq_expr.h"

#endif

