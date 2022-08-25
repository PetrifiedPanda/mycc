#ifndef JUMP_STATEMENT_H
#define JUMP_STATEMENT_H

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct expr;
struct identifier;

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

struct jump_statement* parse_jump_statement(struct parser_state* s);

void free_jump_statement(struct jump_statement* s);

#include "frontend/ast/identifier.h"

#include "frontend/ast/expr/expr.h"

#endif

