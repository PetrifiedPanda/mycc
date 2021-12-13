#ifndef SELECTION_STATEMENT_H
#define SELECTION_STATEMENT_H

#include <stdbool.h>

struct expr;
struct statement;

struct selection_statement {
    bool is_if;
    struct expr* sel_expr;
    struct statement* sel_stat;
    struct statement* else_stat;
};

struct selection_statement* create_if_else_statement(struct expr* sel_expr, struct statement* sel_stat, struct statement* else_stat);
struct selection_statement* create_switch_statement(struct expr* sel_expr, struct statement* sel_stat);

void free_selection_statement(struct selection_statement* s);

#include "ast/expr.h"
#include "ast/statement.h"

#endif

