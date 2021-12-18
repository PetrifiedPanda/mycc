#include "ast/primary_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static struct primary_expr* create_primary_expr_constant(struct constant constant) {
    assert(constant.spelling);

    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_CONSTANT;
    res->constant = constant;
    
    return res;
}

static struct primary_expr* create_primary_expr_string(struct string_constant string) {
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_STRING_LITERAL;
    res->string = string;
    
    return res;
}

static struct primary_expr* create_primary_expr_identifier(struct identifier* identifier) {
    assert(identifier);

    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_IDENTIFIER;
    res->identifier = identifier;

    return res;
}

static struct primary_expr* create_primary_expr_bracket(struct expr* bracket_expr) {
    assert(bracket_expr);
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_BRACKET;
    res->bracket_expr = bracket_expr;
    
    return res;
}

static struct primary_expr* create_primary_expr_generic(struct generic_sel* generic) {
    assert(generic);
    struct primary_expr* res = xmalloc(sizeof(struct primary_expr));
    res->type = PRIMARY_EXPR_GENERIC;
    res->generic = generic;

    return res;
}

struct primary_expr* parse_primary_expr(struct parser_state* s) {
    switch (s->it->type) {
        case IDENTIFIER: {
            char* spelling = take_spelling(s->it);
            accept_it(s);
            if (is_enum_constant(s, spelling)) {
                return create_primary_expr_constant(create_constant(ENUM, spelling));
            }
            return create_primary_expr_identifier(create_identifier(spelling));
        }
        case F_CONSTANT:
        case I_CONSTANT: {
            enum token_type type = s->it->type;
            char* spelling = take_spelling(s->it);
            accept_it(s);
            return create_primary_expr_constant(create_constant(type, spelling));
        }
        case STRING_LITERAL: {
            char* spelling = take_spelling(s->it);
            accept_it(s);
            return create_primary_expr_string(create_string_constant(spelling));
        }
        case FUNC_NAME: {
            accept_it(s);
            return create_primary_expr_string(create_func_name());
        }
        case GENERIC: {
            struct generic_sel* generic = parse_generic_sel(s);
            if (!generic) {
                return NULL;
            }
            return create_primary_expr_generic(generic);
        }

        default:
            if (accept(s, LBRACKET)) {
                struct expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (accept(s, RBRACKET)) {
                    return create_primary_expr_bracket(bracket_expr);
                } else {
                    free_expr(bracket_expr);
                    expected_token_error(RBRACKET, s->it);
                    return NULL;
                }
            } else {
                enum token_type expected[] = {
                        IDENTIFIER,
                        I_CONSTANT,
                        F_CONSTANT,
                        STRING_LITERAL,
                        LBRACKET
                };
                size_t size = sizeof expected / sizeof(enum token_type);
                expected_tokens_error(expected, size, s->it);
                return NULL;
            }
    }

    return NULL;
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

        case PRIMARY_EXPR_GENERIC:
            free_generic_sel(e->generic);
            break;
    }
}

void free_primary_expr(struct primary_expr* e) {
    free_children(e);
    free(e);
}

