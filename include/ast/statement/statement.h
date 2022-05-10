#ifndef STATEMENT_H
#define STATEMENT_H

#include "parser/parser_state.h"

struct labeled_statement;
struct compound_statement;
struct expr_statement;
struct selection_statement;
struct iteration_statement;
struct jump_statement;

enum statement_type {
    STATEMENT_LABELED,
    STATEMENT_COMPOUND,
    STATEMENT_EXPRESSION,
    STATEMENT_SELECTION,
    STATEMENT_ITERATION,
    STATEMENT_JUMP
};

struct statement {
    enum statement_type type;
    union {
        struct labeled_statement* labeled;
        struct compound_statement* comp;
        struct expr_statement* expr;
        struct selection_statement* sel;
        struct iteration_statement* it;
        struct jump_statement* jmp;
    };
};

bool parse_statement_inplace(struct parser_state* s, struct statement* res);

struct statement* parse_statement(struct parser_state* s);

void free_statement_children(struct statement* s);

void free_statement(struct statement* s);

#include "ast/statement/labeled_statement.h"
#include "ast/statement/compound_statement.h"
#include "ast/statement/expr_statement.h"
#include "ast/statement/selection_statement.h"
#include "ast/statement/iteration_statement.h"
#include "ast/statement/jump_statement.h"

#endif
