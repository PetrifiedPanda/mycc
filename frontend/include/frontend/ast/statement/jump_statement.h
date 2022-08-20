#ifndef JUMP_STATEMENT_H
#define JUMP_STATEMENT_H

#include "frontend/token_type.h"

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct expr;
struct identifier;

struct jump_statement {
    struct ast_node_info info;
    enum token_type type;
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

