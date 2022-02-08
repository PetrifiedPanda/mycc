#ifndef SELECTION_STATEMENT_H
#define SELECTION_STATEMENT_H

#include <stdbool.h>

#include "parser/parser_state.h"

struct expr;
struct statement;

struct selection_statement {
    bool is_if;
    struct expr* sel_expr;
    struct statement* sel_stat;
    struct statement* else_stat;
};

struct selection_statement* parse_selection_statement(struct parser_state* s);

void free_selection_statement(struct selection_statement* s);

#include "ast/expr.h"
#include "ast/statement.h"

#endif
