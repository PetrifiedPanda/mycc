#include "frontend/ast/expr/PrimaryExpr.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static PrimaryExpr* create_primary_expr_constant(Constant constant) {
    PrimaryExpr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_CONSTANT;
    res->constant = constant;

    return res;
}

static PrimaryExpr* create_primary_expr_string(
    StringConstant string) {
    PrimaryExpr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_STRING_LITERAL;
    res->string = string;

    return res;
}

static PrimaryExpr* create_primary_expr_identifier(Identifier* identifier) {
    assert(identifier);

    PrimaryExpr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_IDENTIFIER;
    res->identifier = identifier;

    return res;
}

static PrimaryExpr* create_primary_expr_bracket(
    Expr* bracket_expr,
    SourceLoc loc) {
    assert(bracket_expr);
    PrimaryExpr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_BRACKET;
    res->info = create_ast_node_info(loc);
    res->bracket_expr = bracket_expr;

    return res;
}

static PrimaryExpr* create_primary_expr_generic(GenericSel* generic) {
    assert(generic);
    PrimaryExpr* res = mycc_alloc(sizeof *res);
    res->kind = PRIMARY_EXPR_GENERIC;
    res->generic = generic;

    return res;
}

PrimaryExpr* parse_primary_expr(ParserState* s) {
    switch (s->it->kind) {
        case TOKEN_IDENTIFIER: {
            const Str spelling = token_take_spelling(s->it);
            SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            if (parser_is_enum_constant(s, &spelling)) {
                return create_primary_expr_constant(
                    create_enum_constant(&spelling, loc));
            }
            return create_primary_expr_identifier(
                create_identifier(&spelling, loc));
        }
        case TOKEN_F_CONSTANT: {
            const SourceLoc loc = s->it->loc;
            const FloatValue val = s->it->float_val;
            parser_accept_it(s);
            return create_primary_expr_constant(
                create_float_constant(val, loc));
        }
        case TOKEN_I_CONSTANT: {
            const SourceLoc loc = s->it->loc;
            IntValue val = s->it->int_val;
            parser_accept_it(s);
            return create_primary_expr_constant(create_int_constant(val, loc));
        }
        case TOKEN_STRING_LITERAL: {
            const StrLit lit = token_take_str_lit(s->it);
            const SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            return create_primary_expr_string(
                create_string_constant(&lit, loc));
        }
        case TOKEN_FUNC_NAME: {
            const SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            return create_primary_expr_string(create_func_name(loc));
        }
        case TOKEN_GENERIC: {
            GenericSel* generic = parse_generic_sel(s);
            if (!generic) {
                return NULL;
            }
            return create_primary_expr_generic(generic);
        }

        default: {
            const SourceLoc loc = s->it->loc;
            if (parser_accept(s, TOKEN_LBRACKET)) {
                Expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (parser_accept(s, TOKEN_RBRACKET)) {
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

static void free_primary_expr_children(PrimaryExpr* e) {
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

void free_primary_expr(PrimaryExpr* e) {
    free_primary_expr_children(e);
    mycc_free(e);
}
