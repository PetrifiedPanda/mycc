#ifndef JUMP_STATEMENT_H
#define JUMP_STATEMENT_H

#include "token_type.h"

#include "parser/parser_state.h"

struct expr;
struct identifier;

struct jump_statement {
    enum token_type type;
    union {
        struct identifier* identifier;
        struct expr* ret_val;
    };
};

struct jump_statement* parse_jump_statement(struct parser_state* s);

void free_jump_statement(struct jump_statement* s);

#include "ast/identifier.h"

#include "ast/expr/expr.h"

#endif
