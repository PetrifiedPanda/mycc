#include "ast/primary_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

PrimaryExpr* create_primary_expr_constant(char* literal) {
    assert(literal);

    PrimaryExpr* res = xmalloc(sizeof(PrimaryExpr));
    res->type = PRIMARY_EXPR_CONSTANT;
    res->literal = literal;
    
    return res;
}

PrimaryExpr* create_primary_expr_string(char* literal) {
    assert(literal);

    PrimaryExpr* res = xmalloc(sizeof(PrimaryExpr));
    res->type = PRIMARY_EXPR_STRING_LITERAL;
    res->literal = literal;
    
    return res;
}

PrimaryExpr* create_primary_expr_identifier(Identifier* identifier) {
    assert(identifier);

    PrimaryExpr* res = xmalloc(sizeof(PrimaryExpr));
    res->type = PRIMARY_EXPR_IDENTIFIER;
    res->identifier = identifier;

    return res;
}

PrimaryExpr* create_primary_expr_bracket(Expr* bracket_expr) {
    assert(bracket_expr);
    PrimaryExpr* res = xmalloc(sizeof(PrimaryExpr));
    res->type = PRIMARY_EXPR_BRACKET;
    res->bracket_expr = bracket_expr;
    
    return res;
}

static void free_children(PrimaryExpr* e) {
    switch (e->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            free_identifier(e->identifier);
            break;

        case PRIMARY_EXPR_CONSTANT:
        case PRIMARY_EXPR_STRING_LITERAL:
            free(e->literal);
            break;

        case PRIMARY_EXPR_BRACKET:
            free_expr(e->bracket_expr);
            break;
    }
}

void free_primary_expr(PrimaryExpr* e) {
    free_children(e);
    free(e);
}

