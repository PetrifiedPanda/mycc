#include "ast/expr_statement.h"

#include <stdlib.h>

ExprStatement* create_expr_statement(Expr* expr) {
    ExprStatement* res = malloc(sizeof(ExprStatement));
    if (res) {
        res->expr = expr;
    }
    return res;
}

static void free_children(ExprStatement* s) {
    if (s->expr) {
        free_expr(s->expr);
    }
}

void free_expr_statement(ExprStatement* s) {
    free_children(s);
    free(s);
}
