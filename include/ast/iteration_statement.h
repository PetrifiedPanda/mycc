#ifndef ITERATION_STATEMENT_H
#define ITERATION_STATEMENT_H

#include "token_type.h"

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

struct iteration_statement* create_while_loop(struct expr* while_cond, struct statement* loop_body);
struct iteration_statement* create_do_loop(struct expr* while_cond, struct statement* loop_body);
struct iteration_statement* create_for_loop(struct expr_statement* init_expr, struct expr_statement* for_cond, struct expr* incr_expr, struct statement* loop_body);
struct iteration_statement* create_for_loop_decl(struct declaration* decl, struct expr_statement* for_cond, struct expr* incr_expr, struct statement* loop_body);

void free_iteration_statement(struct iteration_statement* s);

#include "ast/declaration.h"
#include "ast/expr_statement.h"
#include "ast/expr.h"
#include "ast/statement.h"

#endif

