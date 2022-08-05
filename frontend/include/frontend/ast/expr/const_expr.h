#ifndef CONST_EXPR_H
#define CONST_EXPR_H

#include "cond_expr.h"

#include "frontend/parser/parser_state.h"

struct const_expr {
    struct cond_expr expr;
};

struct const_expr* parse_const_expr(struct parser_state* s);

void free_const_expr(struct const_expr* e);

#endif
