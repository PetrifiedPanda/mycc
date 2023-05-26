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
    res->info = AstNodeInfo_create(loc);
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
            const Str spelling = Token_take_spelling(s->it);
            SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            if (parser_is_enum_constant(s, &spelling)) {
                return create_primary_expr_constant(
                    Constant_create_enum(&spelling, loc));
            }
            return create_primary_expr_identifier(
                Identifier_create(&spelling, loc));
        }
        case TOKEN_F_CONSTANT:
        case TOKEN_I_CONSTANT: {
            const SourceLoc loc = s->it->loc;
            const Value val = s->it->val;
            parser_accept_it(s);
            return create_primary_expr_constant(Constant_create(val, loc));
        }
        case TOKEN_STRING_LITERAL: {
            const StrLit lit = Token_take_str_lit(s->it);
            const SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            return create_primary_expr_string(
                StringConstant_create(&lit, loc));
        }
        case TOKEN_FUNC_NAME: {
            const SourceLoc loc = s->it->loc;
            parser_accept_it(s);
            return create_primary_expr_string(StringConstant_create_func_name(loc));
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
                    Expr_free(bracket_expr);
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
            Identifier_free(e->identifier);
            break;

        case PRIMARY_EXPR_CONSTANT:
            Constant_free(&e->constant);
            break;

        case PRIMARY_EXPR_STRING_LITERAL:
            StringConstant_free(&e->string);
            break;

        case PRIMARY_EXPR_BRACKET:
            Expr_free(e->bracket_expr);
            break;

        case PRIMARY_EXPR_GENERIC:
            GenericSel_free(e->generic);
            break;
    }
}

void PrimaryExpr_free(PrimaryExpr* e) {
    free_primary_expr_children(e);
    mycc_free(e);
}
