#include "frontend/ast/statement/jump_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static struct jump_statement* create(struct source_loc loc,
                                     enum jump_statement_type type) {
    struct jump_statement* res = xmalloc(sizeof(struct jump_statement));
    res->info = create_ast_node_info(loc);
    res->type = type;

    return res;
}

static struct jump_statement* create_goto_statement(
    struct source_loc loc,
    struct identifier* identifier) {
    assert(identifier);
    struct jump_statement* res = create(loc, JUMP_STATEMENT_GOTO);
    res->goto_label = identifier;

    return res;
}

static struct jump_statement* create_return_statement(struct source_loc loc,
                                                      struct expr* ret_val) {
    struct jump_statement* res = create(loc, JUMP_STATEMENT_RETURN);
    res->ret_val = ret_val;
    return res;
}

struct jump_statement* parse_jump_statement(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    struct jump_statement* res = NULL;
    switch (s->it->type) {
        case GOTO: {
            accept_it(s);
            if (s->it->type == IDENTIFIER) {
                const struct str spell = take_spelling(s->it);
                const struct source_loc id_loc = s->it->loc;
                accept_it(s);
                res = create_goto_statement(loc, create_identifier(&spell, id_loc));
                break;
            } else {
                expected_token_error(s, IDENTIFIER);
                return NULL;
            }
        }
        case CONTINUE:
        case BREAK: {
            const enum token_type t = s->it->type;
            accept_it(s);
            res = create(loc, t == CONTINUE ? JUMP_STATEMENT_CONTINUE : JUMP_STATEMENT_BREAK);
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

            res = create_return_statement(loc, ret_val);
            break;
        }

        default: {
            enum token_type expected[] = {GOTO, CONTINUE, BREAK, RETURN};
            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
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
        case JUMP_STATEMENT_GOTO:
            free_identifier(s->goto_label);
            break;
        case JUMP_STATEMENT_RETURN:
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
