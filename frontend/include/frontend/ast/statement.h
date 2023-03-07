#ifndef STATEMENT_H
#define STATEMENT_H

#include "frontend/parser/parser_state.h"

#include "frontend/ast/declaration/declaration.h"

#include "frontend/ast/expr/expr.h"

#include "frontend/ast/ast_node_info.h"

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

struct const_expr;
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

struct block_item {
    bool is_decl;
    union {
        struct declaration decl;
        struct statement stat;
    };
};

struct compound_statement {
    struct ast_node_info info;
    size_t len;
    struct block_item* items;
};

struct expr_statement {
    struct ast_node_info info;
    struct expr expr;
};

struct selection_statement {
    struct ast_node_info info;
    bool is_if;
    struct expr* sel_expr;
    struct statement* sel_stat;
    struct statement* else_stat;
};

struct for_loop {
    bool is_decl;
    union {
        struct declaration init_decl;
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

enum jump_statement_type {
    JUMP_STATEMENT_GOTO,
    JUMP_STATEMENT_CONTINUE,
    JUMP_STATEMENT_BREAK,
    JUMP_STATEMENT_RETURN,
};

struct jump_statement {
    struct ast_node_info info;
    enum jump_statement_type type;
    union {
        struct identifier* goto_label;
        struct expr* ret_val;
    };
};

bool parse_statement_inplace(struct parser_state* s, struct statement* res);
struct statement* parse_statement(struct parser_state* s);

bool parse_compound_statement_inplace(struct parser_state* s,
                                      struct compound_statement* res);

void free_statement_children(struct statement* s);
void free_statement(struct statement* s);

void free_labeled_statement(struct labeled_statement* s);

void free_compound_statement(struct compound_statement* s);
void free_compound_statement_children(struct compound_statement* s);

void free_expr_statement(struct expr_statement* s);

void free_selection_statement(struct selection_statement* s);

void free_iteration_statement(struct iteration_statement* s);

void free_jump_statement(struct jump_statement* s);

#include "frontend/ast/expr/const_expr.h"
#include "frontend/ast/identifier.h"

#endif

