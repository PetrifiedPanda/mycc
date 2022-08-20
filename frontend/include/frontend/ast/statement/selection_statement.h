#ifndef SELECTION_STATEMENT_H
#define SELECTION_STATEMENT_H

#include <stdbool.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct expr;
struct statement;

struct selection_statement {
    struct ast_node_info info;
    bool is_if;
    struct expr* sel_expr;
    struct statement* sel_stat;
    struct statement* else_stat;
};

struct selection_statement* parse_selection_statement(struct parser_state* s);

void free_selection_statement(struct selection_statement* s);

#include "statement.h"

#include "frontend/ast/expr/expr.h"

#endif

