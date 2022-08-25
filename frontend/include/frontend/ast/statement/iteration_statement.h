#ifndef ITERATION_STATEMENT_H
#define ITERATION_STATEMENT_H

#include "frontend/token_type.h"

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

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

enum iteration_statement_type {
    ITERATION_STATEMENT_WHILE,
    ITERATION_STATEMENT_DO,
    ITERATION_STATEMENT_FOR,
};

struct iteration_statement {
    struct ast_node_info info;
    enum iteration_statement_type type;
    struct statement* loop_body;
    union {
        struct expr* while_cond;
        struct for_loop for_loop;
    };
};

struct iteration_statement* parse_iteration_statement(struct parser_state* s);

void free_iteration_statement(struct iteration_statement* s);

#include "expr_statement.h"
#include "statement.h"

#include "frontend/ast/declaration/declaration.h"

#include "frontend/ast/expr/expr.h"

#endif

