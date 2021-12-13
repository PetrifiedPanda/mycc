#ifndef JUMP_STATEMENT_H
#define JUMP_STATEMENT_H

#include "token_type.h"

struct expr;
struct identifier;

struct jump_statement {
    enum token_type type;
    union {
        struct identifier* identifier;
        struct expr* ret_val;
    };
};

struct jump_statement* create_goto_statement(struct identifier* identifier);
struct jump_statement* create_continue_statement();
struct jump_statement* create_break_statement();
struct jump_statement* create_return_statement(struct expr* ret_val);

void free_jump_statement(struct jump_statement* s);

#include "ast/expr.h"
#include "ast/identifier.h"

#endif

