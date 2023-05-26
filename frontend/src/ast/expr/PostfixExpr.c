#include "frontend/ast/expr/PostfixExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool is_posfix_op(TokenKind t) {
    switch (t) {
        case TOKEN_LINDEX:
        case TOKEN_LBRACKET:
        case TOKEN_DOT:
        case TOKEN_PTR_OP:
        case TOKEN_INC:
        case TOKEN_DEC:
            return true;

        default:
            return false;
    }
}

static bool parse_postfix_arr_suffix(ParserState* s, PostfixSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LINDEX);

    parser_accept_it(s);
    Expr* expr = parse_expr(s);
    if (!expr) {
        return false;
    }
    if (!parser_accept(s, TOKEN_RINDEX)) {
        Expr_free(expr);
        return false;
    }

    res->kind = POSTFIX_INDEX;
    res->index_expr = expr;
    return true;
}

static bool parse_postfix_func_suffix(ParserState* s, PostfixSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LBRACKET);

    parser_accept_it(s);
    ArgExprList arg_expr_list = {
        .assign_exprs = NULL,
        .len = 0,
    };
    if (s->it->kind != TOKEN_RBRACKET) {
        if (!parse_arg_expr_list(s, &arg_expr_list)) {
            return false;
        }
    }
    if (!parser_accept(s, TOKEN_RBRACKET)) {
        ArgExprList_free(&arg_expr_list);
        return false;
    }
    res->kind = POSTFIX_BRACKET;
    res->bracket_list = arg_expr_list;
    return true;
}

static bool parse_postfix_access_suffix(ParserState* s, PostfixSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_DOT || s->it->kind == TOKEN_PTR_OP);
    PostfixSuffixKind kind = s->it->kind == TOKEN_PTR_OP
                                        ? POSTFIX_PTR_ACCESS
                                        : POSTFIX_ACCESS;
    parser_accept_it(s);
    if (s->it->kind != TOKEN_IDENTIFIER) {
        return false;
    }
    const Str spelling = Token_take_spelling(s->it);
    const SourceLoc loc = s->it->loc;
    parser_accept_it(s);
    Identifier* identifier = Identifier_create(&spelling, loc);
    res->kind = kind;
    res->identifier = identifier;
    return true;
}

PostfixSuffix parse_postfix_inc_dec_suffix(ParserState* s) {
    assert(s->it->kind == TOKEN_INC || s->it->kind == TOKEN_DEC);
    const TokenKind op = s->it->kind;
    parser_accept_it(s);
    return (PostfixSuffix){
        .kind = op == TOKEN_INC ? POSTFIX_INC : POSTFIX_DEC,
    };
}

static bool parse_postfix_suffixes(ParserState* s, PostfixExpr* res) {
    size_t alloc_len = 0;
    while (is_posfix_op(s->it->kind)) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->suffixes,
                            &alloc_len,
                            sizeof *res->suffixes);
        }

        switch (s->it->kind) {
            case TOKEN_LINDEX:
                if (!parse_postfix_arr_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case TOKEN_LBRACKET:
                if (!parse_postfix_func_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case TOKEN_DOT:
            case TOKEN_PTR_OP:
                if (!parse_postfix_access_suffix(s, &res->suffixes[res->len])) {
                    return false;
                }
                break;

            case TOKEN_INC:
            case TOKEN_DEC:
                res->suffixes[res->len] = parse_postfix_inc_dec_suffix(s);
                break;

            default:
                UNREACHABLE();
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->suffixes = mycc_realloc(res->suffixes,
                                     sizeof *res->suffixes * res->len);
    }

    return true;
}

PostfixExpr* parse_postfix_expr(ParserState* s) {
    PostfixExpr* res = mycc_alloc(sizeof *res);
    res->suffixes = NULL;
    res->len = 0;

    if (s->it->kind == TOKEN_LBRACKET && next_is_type_name(s)) {
        res->info = AstNodeInfo_create(s->it->loc);
        parser_accept_it(s);

        res->is_primary = false;

        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            mycc_free(res);
            return NULL;
        }

        if (!(parser_accept(s, TOKEN_RBRACKET)
              && parser_accept(s, TOKEN_LBRACE))) {
            TypeName_free(res->type_name);
            mycc_free(res);
            return NULL;
        }

        if (!parse_init_list(s, &res->init_list)) {
            TypeName_free(res->type_name);
            mycc_free(res);
            return NULL;
        }

        if (s->it->kind == TOKEN_COMMA) {
            parser_accept_it(s);
        }

        if (!parser_accept(s, TOKEN_RBRACE)) {
            PostfixExpr_free(res);
            return NULL;
        }
    } else {
        res->is_primary = true;
        res->primary = parse_primary_expr(s);
        if (!res->primary) {
            mycc_free(res);
            return NULL;
        }
    }

    if (!parse_postfix_suffixes(s, res)) {
        PostfixExpr_free(res);
        return NULL;
    }

    return res;
}

/**
 *
 * @param s current state
 * @param type_name A type name that was already parsed by parse_unary_expr
 * @return A postfix_expr that uses the given type_name
 */
PostfixExpr* parse_postfix_expr_type_name(ParserState* s, TypeName* type_name, SourceLoc start_bracket_loc) {
    assert(type_name);
    assert(s->it->kind == TOKEN_LBRACE);

    PostfixExpr* res = mycc_alloc(sizeof *res);
    res->len = 0;
    res->suffixes = NULL;
    res->is_primary = false;
    res->info = AstNodeInfo_create(start_bracket_loc);
    res->type_name = type_name;

    res->init_list.len = 0;
    res->init_list.inits = NULL;

    parser_accept_it(s);

    if (!parse_init_list(s, &res->init_list)) {
        goto fail;
    }

    if (s->it->kind == TOKEN_COMMA) {
        parser_accept_it(s);
    }

    if (!parser_accept(s, TOKEN_RBRACE)) {
        return NULL;
    }

    if (!parse_postfix_suffixes(s, res)) {
        goto fail;
    }
    return res;
fail:
    PostfixExpr_free(res);
    return NULL;
}

static void free_postfix_expr_children(PostfixExpr* p) {
    if (p->is_primary) {
        PrimaryExpr_free(p->primary);
    } else {
        TypeName_free(p->type_name);
        InitList_free_children(&p->init_list);
    }
    for (size_t i = 0; i < p->len; ++i) {
        PostfixSuffix* s = &p->suffixes[i];
        switch (s->kind) {
            case POSTFIX_INDEX:
                Expr_free(s->index_expr);
                break;
            case POSTFIX_BRACKET:
                ArgExprList_free(&s->bracket_list);
                break;
            case POSTFIX_ACCESS:
            case POSTFIX_PTR_ACCESS:
                Identifier_free(s->identifier);
                break;
            case POSTFIX_INC:
            case POSTFIX_DEC:
                break;
        }
    }
    mycc_free(p->suffixes);
}

void PostfixExpr_free(PostfixExpr* p) {
    free_postfix_expr_children(p);
    mycc_free(p);
}

