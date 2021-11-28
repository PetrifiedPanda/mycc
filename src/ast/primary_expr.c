#include "ast/primary_expr.h"

#include "ast/ast_common.h"

#include <stdlib.h>
#include <assert.h>

PrimaryExpr* create_primary_expr(TokenType type, char* spelling) {
    assert(spelling);
    assert(type == IDENTIFIER || type == CONSTANT || type == STRING_LITERAL);
    PrimaryExpr* res = malloc(sizeof(PrimaryExpr));
    if (res) {
        res->is_bracket = false;
        res->type = type;
        res->spelling = spelling;
    } else {
        ast_alloc_fail();
    }
    return res;
}

PrimaryExpr* create_primary_expr_bracket(Expr* bracket_expr) {
    assert(bracket_expr);
    PrimaryExpr* res = malloc(sizeof(PrimaryExpr));
    if (res) {
        res->is_bracket = true;
        res->bracket_expr = bracket_expr;
    } else {
        ast_alloc_fail();
    }
    return res;
}

static void free_children(PrimaryExpr* e) {
    if (e->is_bracket) {
        free_expr(e->bracket_expr);
    } else {
        free(e->spelling);
    }
}

void free_primary_expr(PrimaryExpr* e) {
    free_children(e);
    free(e);
}

