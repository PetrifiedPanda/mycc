#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct assign_expr;

struct expr {
    size_t len;
    struct assign_expr* assign_exprs;
};

struct expr* parse_expr(struct parser_state* s);

void free_expr_children(struct expr* expr);

void free_expr(struct expr* expr);

#include "ast/assign_expr.h"

#endif

