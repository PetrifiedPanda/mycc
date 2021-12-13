#include "ast/expr_statement.h"

#include <stdlib.h>

#include "util.h"

struct expr_statement* create_expr_statement(struct expr expr) {
    struct expr_statement* res = xmalloc(sizeof(struct expr_statement));
    res->expr = expr;

    return res;
}

static void free_children(struct expr_statement* s) {
    free_expr_children(&s->expr);
}

void free_expr_statement(struct expr_statement* s) {
    free_children(s);
    free(s);
}

