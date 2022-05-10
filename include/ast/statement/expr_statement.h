#ifndef EXPR_STATEMENT_H
#define EXPR_STATEMENT_H

#include "ast/expr/expr.h"

#include "parser/parser_state.h"

struct expr_statement {
    struct expr expr;
};

struct expr_statement* parse_expr_statement(struct parser_state* s);

void free_expr_statement(struct expr_statement* s);

#endif
