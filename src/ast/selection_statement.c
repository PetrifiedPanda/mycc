#include "ast/selection_statement.h"

#include <stdlib.h>

#include "util.h"

#include "parser/parser_util.h"

struct selection_statement* parse_selection_statement(struct parser_state* s) {
    struct selection_statement* res = xmalloc(
        sizeof(struct selection_statement));
    if (s->it->type == IF) {
        res->is_if = true;
        accept_it(s);
    } else {
        if (!accept(s, SWITCH)) {
            free(res);
            enum token_type expected[] = {IF, SWITCH};

            expected_tokens_error(expected,
                                  sizeof expected / sizeof(enum token_type),
                                  s->it);
            return NULL;
        }
        res->is_if = false;
    }

    if (!accept(s, LBRACKET)) {
        free(res);
        return NULL;
    }

    res->sel_expr = parse_expr(s);
    if (!res->sel_expr) {
        free(res);
        return NULL;
    }

    if (!accept(s, RBRACKET)) {
        free_expr(res->sel_expr);
        free(res);
        return NULL;
    }

    res->sel_stat = parse_statement(s);
    if (!res->sel_stat) {
        free_expr(res->sel_expr);
        free(res);
        return NULL;
    }

    if (res->is_if && s->it->type == ELSE) {
        accept_it(s);
        res->else_stat = parse_statement(s);
        if (!res->else_stat) {
            free_statement(res->sel_stat);
            free_expr(res->sel_expr);
            free(res);
            return NULL;
        }
    } else {
        res->else_stat = NULL;
    }

    return res;
}

static void free_children(struct selection_statement* s) {
    free_expr(s->sel_expr);
    free_statement(s->sel_stat);
    if (s->else_stat) {
        free_statement(s->else_stat);
    }
}

void free_selection_statement(struct selection_statement* s) {
    free_children(s);
    free(s);
}
