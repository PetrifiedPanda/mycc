#include "ast/statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

bool parse_statement_inplace(struct parser_state* s, struct statement* res) {
    assert(res);

    switch (s->it->type) {
        case LBRACE: {
            res->type = STATEMENT_COMPOUND;
            res->comp = parse_compound_statement(s);
            if (!res->comp) {
                return false;
            }
            break;
        }

        case FOR:
        case WHILE:
        case DO: {
            res->type = STATEMENT_ITERATION;
            res->it = parse_iteration_statement(s);
            if (!res->it) {
                return false;
            }
            break;
        }

        case GOTO:
        case CONTINUE:
        case BREAK:
        case RETURN: {
            res->type = STATEMENT_JUMP;
            res->jmp = parse_jump_statement(s);
            if (!res->jmp) {
                return false;
            }
            break;
        }

        case IF:
        case SWITCH: {
            res->type = STATEMENT_SELECTION;
            res->sel = parse_selection_statement(s);
            if (!res->sel) {
                return false;
            }
            break;
        }

        case CASE:
        case DEFAULT: {
            res->type = STATEMENT_LABELED;
            res->labeled = parse_labeled_statement(s);
            if (!res->labeled) {
                return false;
            }
            break;
        }

        case IDENTIFIER: {
            if (s->it[1].type == COLON) {
                res->type = STATEMENT_LABELED;
                res->labeled = parse_labeled_statement(s);
                if (!res->labeled) {
                    return false;
                }
                break;
            }
            // FALLTHROUGH
        }

        default: {
            res->type = STATEMENT_EXPRESSION;
            res->expr = parse_expr_statement(s);
            if (!res->expr) {
                return false;
            }
        }
    }
    return true;
}

struct statement* parse_statement(struct parser_state* s) {
    struct statement* res = xmalloc(sizeof(struct statement));
    if (!parse_statement_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_statement_children(struct statement* s) {
    switch (s->type) {
        case STATEMENT_LABELED:
            free_labeled_statement(s->labeled);
            break;
        case STATEMENT_COMPOUND:
            free_compound_statement(s->comp);
            break;
        case STATEMENT_EXPRESSION:
            free_expr_statement(s->expr);
            break;
        case STATEMENT_SELECTION:
            free_selection_statement(s->sel);
            break;
        case STATEMENT_ITERATION:
            free_iteration_statement(s->it);
            break;
        case STATEMENT_JUMP:
            free_jump_statement(s->jmp);
            break;
    }
}

void free_statement(struct statement* s) {
    free_statement_children(s);
    free(s);
}

