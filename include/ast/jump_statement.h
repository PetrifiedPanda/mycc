#ifndef JUMP_STATEMENT_H
#define JUMP_STATEMENT_H

#include "token_type.h"

typedef struct Expr Expr;
typedef struct Identifier Identifier;

typedef struct JumpStatement {
    TokenType type;
    union {
        Identifier* identifier;
        Expr* ret_val;
    };
} JumpStatement;

JumpStatement* create_goto_statement(Identifier* identifier);
JumpStatement* create_continue_statement();
JumpStatement* create_break_statement();
JumpStatement* create_return_statement(Expr* ret_val);

void free_jump_statement(JumpStatement* s);

#include "ast/expr.h"
#include "ast/identifier.h"

#endif

