#ifndef ITERATION_STATEMENT_H
#define ITERATION_STATEMENT_H

#include "token_type.h"

#include "parser/parser_state.h"

struct declaration;
struct expr_statement;
struct expr;
struct statement;

struct for_loop {
    bool is_decl;
    union {
        struct declaration* init_decl;
        struct expr_statement* init_expr;
    };
    struct expr_statement* cond;
    struct expr* incr_expr;
};

struct iteration_statement {
    enum token_type type;
    struct statement* loop_body;
    union {
        struct expr* while_cond;
        struct for_loop for_loop;
    };
};

struct iteration_statement* parse_iteration_statement(struct parser_state* s);

void free_iteration_statement(struct iteration_statement* s);

#include "ast/statement/expr_statement.h"
#include "ast/statement/statement.h"

#include "ast/declaration/declaration.h"

#include "ast/expr/expr.h"

#endif

