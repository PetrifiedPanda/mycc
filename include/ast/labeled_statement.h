#ifndef LABELED_STATEMENT_H
#define LABELED_STATEMENT_H

#include "token_type.h"

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

struct labeled_statement* create_labeled_statement_goto(struct identifier* identifier, struct statement* stat);
struct labeled_statement* create_labeled_statement_case(struct const_expr* case_expr, struct statement* stat);
struct labeled_statement* craete_labeled_statement_default(struct statement* stat);

void free_labeled_statement(struct labeled_statement* s);

#include "ast/const_expr.h"
#include "ast/statement.h"
#include "ast/identifier.h"

#endif

