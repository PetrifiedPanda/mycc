#include "ast/jump_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static struct jump_statement* create(enum token_type type) {
    assert(type == GOTO || type == CONTINUE || type == BREAK || type == RETURN);
    struct jump_statement* res = xmalloc(sizeof(struct jump_statement));
    res->type = type;
    
    return res;
} 

struct jump_statement* create_goto_statement(struct identifier* identifier) {
    assert(identifier);
    struct jump_statement* res = create(GOTO);
    res->identifier = identifier;

    return res;
}

struct jump_statement* create_continue_statement() {
    return create(CONTINUE);
}

struct jump_statement* create_break_statement() {
    return create(BREAK);
}

struct jump_statement* create_return_statement(struct expr* ret_val) {
    struct jump_statement* res = create(RETURN);
    res->ret_val = ret_val;
    return res;
}

static void free_children(struct jump_statement* s) {
    switch (s->type) {
        case GOTO:
            free_identifier(s->identifier);
            break;
        case RETURN:
            if (s->ret_val) {
                free_expr(s->ret_val);
            }
            break;
        default:
            assert(false);
    }
}

void free_jump_statement(struct jump_statement* s) {
    free_children(s);
    free(s);
}

