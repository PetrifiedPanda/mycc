#ifndef LABELED_STATEMENT_H
#define LABELED_STATEMENT_H

#include "token_type.h"

#include "parser/parser_state.h"

struct const_expr;
struct statement;
struct identifier;

struct labeled_statement {
    enum token_type type;
    union {
        struct identifier* identifier;
        struct const_expr* case_expr;
    };
    struct statement* stat;
};

struct labeled_statement* parse_labeled_statement(struct parser_state* s);

void free_labeled_statement(struct labeled_statement* s);

#include "ast/statement/statement.h"

#include "ast/identifier.h"

#include "ast/expr/const_expr.h"

#endif
