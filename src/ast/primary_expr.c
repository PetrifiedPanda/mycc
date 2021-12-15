#include "ast/primary_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct primary_expr* create_primary_expr_constant(struct constant constant) {
    assert(constant.spelling);

    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_CONSTANT;
    res->constant = constant;
    
    return res;
}

struct primary_expr* create_primary_expr_string(struct string_constant string) {
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_STRING_LITERAL;
    res->string = string;
    
    return res;
}

struct primary_expr* create_primary_expr_identifier(struct identifier* identifier) {
    assert(identifier);

    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_IDENTIFIER;
    res->identifier = identifier;

    return res;
}

struct primary_expr* create_primary_expr_bracket(struct expr* bracket_expr) {
    assert(bracket_expr);
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_BRACKET;
    res->bracket_expr = bracket_expr;
    
    return res;
}

static void free_children(struct primary_expr* e) {
    switch (e->type) {
        case PRIMARY_EXPR_IDENTIFIER:
            free_identifier(e->identifier);
            break;

        case PRIMARY_EXPR_CONSTANT:
            free_constant(&e->constant);
            break;

        case PRIMARY_EXPR_STRING_LITERAL:
            free_string_constant(&e->string);
            break;

        case PRIMARY_EXPR_BRACKET:
            free_expr(e->bracket_expr);
            break;
    }
}

void free_primary_expr(struct primary_expr* e) {
    free_children(e);
    free(e);
}

