#include "frontend/ast/expr/primary_expr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static struct primary_expr* create_primary_expr_constant(
    struct constant constant) {
    struct primary_expr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_CONSTANT;
    res->constant = constant;

    return res;
}

static struct primary_expr* create_primary_expr_string(
    struct string_constant string) {
    struct primary_expr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_STRING_LITERAL;
    res->string = string;

    return res;
}

static struct primary_expr* create_primary_expr_identifier(
    struct identifier* identifier) {
    assert(identifier);

    struct primary_expr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_IDENTIFIER;
    res->identifier = identifier;

    return res;
}

static struct primary_expr* create_primary_expr_bracket(
    struct expr* bracket_expr,
    struct source_loc loc) {
    assert(bracket_expr);
    struct primary_expr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_BRACKET;
    res->info = create_ast_node_info(loc);
    res->bracket_expr = bracket_expr;

    return res;
}

static struct primary_expr* create_primary_expr_generic(
    struct generic_sel* generic) {
    assert(generic);
    struct primary_expr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_GENERIC;
    res->generic = generic;

    return res;
}

struct primary_expr* parse_primary_expr(struct parser_state* s) {
    switch (s->it->kind) {
        case IDENTIFIER: {
            const struct str spelling = token_take_spelling(s->it);
            struct source_loc loc = s->it->loc;
            accept_it(s);
            if (is_enum_constant(s, &spelling)) {
                return create_primary_expr_constant(
                    create_enum_constant(&spelling, loc));
            }
            return create_primary_expr_identifier(
                create_identifier(&spelling, loc));
        }
        case F_CONSTANT: {
            const struct source_loc loc = s->it->loc;
            const struct float_value val = s->it->float_val;
            accept_it(s);
            return create_primary_expr_constant(
                create_float_constant(val, loc));
        }
        case I_CONSTANT: {
            struct source_loc loc = s->it->loc;
            struct int_value val = s->it->int_val;
            accept_it(s);
            return create_primary_expr_constant(create_int_constant(val, loc));
        }
        case STRING_LITERAL: {
            const struct str_lit lit = token_take_str_lit(s->it);
            struct source_loc loc = s->it->loc;
            accept_it(s);
            return create_primary_expr_string(
                create_string_constant(&lit, loc));
        }
        case FUNC_NAME: {
            struct source_loc loc = s->it->loc;
            accept_it(s);
            return create_primary_expr_string(create_func_name(loc));
        }
        case GENERIC: {
            struct generic_sel* generic = parse_generic_sel(s);
            if (!generic) {
                return NULL;
            }
            return create_primary_expr_generic(generic);
        }

        default: {
            struct source_loc loc = s->it->loc;
            if (accept(s, LBRACKET)) {
                struct expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (accept(s, RBRACKET)) {
                    return create_primary_expr_bracket(bracket_expr, loc);
                } else {
                    free_expr(bracket_expr);
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

static void free_children(struct primary_expr* e) {
    switch (e->kind) {
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
    mycc_free(e);
}
