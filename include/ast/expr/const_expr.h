#ifndef CONST_EXPR_H
#define CONST_EXPR_H

#include "ast/expr/cond_expr.h"

#include "parser/parser_state.h"

struct const_expr {
    struct cond_expr expr;
};

struct const_expr* parse_const_expr(struct parser_state* s);

void free_const_expr(struct const_expr* e);

struct ast_visitor;

bool visit_const_expr(struct ast_visitor* visitor, struct const_expr* e);

#endif

