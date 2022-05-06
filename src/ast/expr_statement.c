#include "ast/expr_statement.h"

#include <stdlib.h>

#include "util/mem.h"

#include "parser/parser_util.h"

struct expr_statement* parse_expr_statement(struct parser_state* s) {
    struct expr_statement* res = xmalloc(sizeof(struct expr_statement));
    if (s->it->type == SEMICOLON) {
        accept_it(s);
        res->expr.len = 0;
        res->expr.assign_exprs = NULL;
        return res;
    } else {
        if (!parse_expr_inplace(s, &res->expr)) {
            free(res);
            return NULL;
        }

        if (!accept(s, SEMICOLON)) {
            free_expr_statement(res);
            return NULL;
        }

        return res;
    }
}

static void free_children(struct expr_statement* s) {
    free_expr_children(&s->expr);
}

void free_expr_statement(struct expr_statement* s) {
    free_children(s);
    free(s);
}
