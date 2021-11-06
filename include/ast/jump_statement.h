#ifndef JUMP_STATEMENT_H
#define JUMP_STATEMENT_H

#include "token_type.h"

typedef struct Expr Expr;

typedef struct JumpStatement {
    TokenType type;
    union {
        char* identifier;
        Expr* ret_val;
    };
} JumpStatement;

JumpStatement* create_goto_statement(char* identifier);
JumpStatement* create_continue_statement();
JumpStatement* create_break_statement();
JumpStatement* create_return_statement(Expr* ret_val);

void free_jump_statement(JumpStatement* s);

#include "ast/expr.h"

#endif

