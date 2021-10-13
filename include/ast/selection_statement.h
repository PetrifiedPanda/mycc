#ifndef SELECTION_STATEMENT_H
#define SELECTION_STATEMENT_H

#include <stdbool.h>

typedef struct Expr Expr;
typedef struct Statement Statement;

typedef struct SelectionStatement {
    bool is_if;
    Expr* sel_expr;
    Statement* sel_stat;
    Statement* else_stat;
} SelectionStatement;

SelectionStatement* create_if_else_statement(Expr* sel_expr, Statement* sel_stat, Statement* else_stat);
SelectionStatement* create_switch_statement(Expr* sel_expr, Statement* sel_stat);

void free_selection_statement(SelectionStatement* s);

#include "ast/expr.h"
#include "ast/statement.h"

#endif
