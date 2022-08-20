#ifndef EXPR_STATEMENT_H
#define EXPR_STATEMENT_H

#include "frontend/ast/expr/expr.h"

#include "frontend/parser/parser_state.h"

struct expr_statement {
    struct ast_node_info info;
    struct expr expr;
};

struct expr_statement* parse_expr_statement(struct parser_state* s);

void free_expr_statement(struct expr_statement* s);

#endif

