#ifndef LABELED_STATEMENT_H
#define LABELED_STATEMENT_H

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct const_expr;
struct statement;
struct identifier;

enum labeled_statement_type {
    LABELED_STATEMENT_CASE,
    LABELED_STATEMENT_LABEL,
    LABELED_STATEMENT_DEFAULT,
};

struct labeled_statement {
    struct ast_node_info info;
    enum labeled_statement_type type;
    union {
        struct identifier* label;
        struct const_expr* case_expr;
    };
    struct statement* stat;
};

struct labeled_statement* parse_labeled_statement(struct parser_state* s);

void free_labeled_statement(struct labeled_statement* s);

#include "statement.h"

#include "frontend/ast/identifier.h"

#include "frontend/ast/expr/const_expr.h"

#endif

