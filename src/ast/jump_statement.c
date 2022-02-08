#include "ast/jump_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static struct jump_statement* create(enum token_type type) {
    assert(type == GOTO || type == CONTINUE || type == BREAK || type == RETURN);
    struct jump_statement* res = xmalloc(sizeof(struct jump_statement));
    res->type = type;

    return res;
}

static struct jump_statement* create_goto_statement(
    struct identifier* identifier) {
    assert(identifier);
    struct jump_statement* res = create(GOTO);
    res->identifier = identifier;

    return res;
}

static struct jump_statement* create_return_statement(struct expr* ret_val) {
    struct jump_statement* res = create(RETURN);
    res->ret_val = ret_val;
    return res;
}

struct jump_statement* parse_jump_statement(struct parser_state* s) {
    struct jump_statement* res = NULL;
    switch (s->it->type) {
        case GOTO: {
            accept_it(s);
            if (s->it->type == IDENTIFIER) {
                char* spell = take_spelling(s->it);
                accept_it(s);
                res = create_goto_statement(create_identifier(spell));
                break;
            } else {
                expected_token_error(IDENTIFIER, s->it);
                return NULL;
            }
        }
        case CONTINUE:
        case BREAK: {
            enum token_type t = s->it->type;
            accept_it(s);
            res = create(t);
            res->ret_val = NULL;
            break;
        }
        case RETURN: {
            accept_it(s);
            struct expr* ret_val;
            if (s->it->type == SEMICOLON) {
                ret_val = NULL;
            } else {
                ret_val = parse_expr(s);
                if (!ret_val) {
                    return NULL;
                }
            }

            res = create_return_statement(ret_val);
            break;
        }

        default: {
            enum token_type expected[] = {GOTO, CONTINUE, BREAK, RETURN};
            expected_tokens_error(expected,
                                  sizeof expected / sizeof(enum token_type),
                                  s->it);
            return NULL;
        }
    }

    if (!accept(s, SEMICOLON)) {
        free_jump_statement(res);
        return NULL;
    }

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
            break;
    }
}

void free_jump_statement(struct jump_statement* s) {
    free_children(s);
    free(s);
}
