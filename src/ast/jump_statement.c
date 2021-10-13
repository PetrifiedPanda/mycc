#include "ast/jump_statement.h"

#include <stdlib.h>
#include <assert.h>

static JumpStatement* create(TokenType type) {
    assert(type == GOTO || type == CONTINUE || type == BREAK || type == RETURN);
    JumpStatement* res = malloc(sizeof(JumpStatement));
    if (res) {
        res->type = type;
    }
    return res;
} 

JumpStatement* create_goto_statement(char* identifier) {
    assert(identifier);
    JumpStatement* res = create(GOTO);
    if (res) {
        res->identifier = identifier;
    }
}

JumpStatement* create_continue_statement() {
    return create(CONTINUE);
}

JumpStatement* create_break_statement() {
    return create(BREAK);
}

JumpStatement* create_return_statement(Expr* ret_val) {
    JumpStatement* res = create(RETURN);
    if (res) {
        res->ret_val = ret_val;
    }
    return res;
}

static void free_children(JumpStatement* s) {
    switch (s->type) {
        case GOTO:
            free(s->identifier);
            break;
        case RETURN:
            if (s->ret_val) {
                free_expr(s->ret_val);
            }
            break;
    }
}

void free_jump_statement(JumpStatement* s) {
    free_children(s);
    free(s);
}
