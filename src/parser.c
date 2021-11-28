#include "parser.h"

#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "util.h"

typedef struct {
    const Token* it;
} ParserState;

static TranslationUnit* parse_translation_unit(ParserState* s);

TranslationUnit* parse_tokens(const Token* tokens) {
    ParserState state = {tokens};
    TranslationUnit* res = parse_translation_unit(&state);
    assert(state.it->type == INVALID);
    return res;
}

static void expected_token_error(TokenType expected, const Token* got) {
    set_error_file(ERR_PARSER, got->file, got->source_loc, "Expected token of type %s but got token of type %s", get_type_str(expected), get_type_str(got->type));
}

static bool accept(ParserState* s, TokenType expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

static void accept_it(ParserState* s) {
    ++s->it;
}

static bool parse_external_declaration(ParserState* s, ExternalDeclaration* res) {
    // TODO:
    return false;
}

static TranslationUnit* parse_translation_unit(ParserState* s) {
    TranslationUnit* res = xmalloc(sizeof(TranslationUnit));
    size_t alloc_num = 1;
    res->len = 0;
    res->external_decls = xmalloc(sizeof(ExternalDeclaration) * alloc_num);

    while (s->it->type != INVALID) {
        if (res->len == alloc_num) {
            grow_alloc((void**)res->external_decls, &alloc_num, sizeof(ExternalDeclaration));
        }
        
        if (!parse_external_declaration(s, &res->external_decls[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    
    if (res->len != alloc_num) {
        res->external_decls = xrealloc(res->external_decls, res->len * sizeof(ExternalDeclaration));
    }

    return res;

fail:
    if (res) {
        free_translation_unit(res);
    }
    return NULL;
}

static bool parse_assign_expr(AssignExpr* res, ParserState* s) {
    return false;
}

static Expr* parse_expr(ParserState* s) {
    size_t num_elems = 1;
    Expr* res = xmalloc(sizeof(Expr));
    res->assign_exprs = xmalloc(num_elems * sizeof(AssignExpr));

    if (parse_assign_expr(&res->assign_exprs[0], s)) {
        res->len = 0;
        goto fail;
    }

    res->len = 1;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (num_elems == res->len) {
            grow_alloc((void**)&res->assign_exprs, &num_elems, sizeof(AssignExpr));
        }

        if (!parse_assign_expr(&res->assign_exprs[res->len], s)) {
            goto fail;
        }

        ++res->len;
    }

    if (num_elems != res->len) {
        res->assign_exprs = xrealloc(res->assign_exprs, sizeof(AssignExpr) * res->len);
    }

    return res;
fail:
    free_expr(res);
    return NULL;
}

static PrimaryExpr* parse_primary_expr(ParserState* s) {
    switch (s->it->type) {
        case IDENTIFIER:
        case CONSTANT:
        case STRING_LITERAL:
            return create_primary_expr(s->it->type, s->it->spelling);
    
        default:
            if (accept(s, LBRACKET)) {
                Expr* bracket_expr = parse_expr(s);
                if (!bracket_expr) {
                    return NULL;
                }
                if (accept(s, RBRACKET)) {
                    return create_primary_expr_bracket(bracket_expr);
                } else {
                    free_expr(bracket_expr);
                    return NULL;
                }
            }
            break;
    }

    // Unreachable
    return NULL;
}