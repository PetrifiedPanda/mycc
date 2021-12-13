#ifndef EXPR_STATEMENT_H
#define EXPR_STATEMENT_H

#include "ast/expr.h"

struct expr_statement {
    struct expr expr;
};

struct expr_statement* create_expr_statement(struct expr expr);

void free_expr_statement(struct expr_statement* s);

#endif
