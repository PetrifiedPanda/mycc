#include "frontend/ast/declaration/DirectAbsDeclarator.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/AssignExpr.h"

#include "frontend/ast/declaration/AbsDeclarator.h"

static void AbsArrOrFuncSuffix_free(AbsArrOrFuncSuffix* s);

static bool parse_abs_func_suffix(ParserState* s, AbsArrOrFuncSuffix* res) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);
    ParserState_accept_it(s);
    res->kind = ABS_ARR_OR_FUNC_SUFFIX_FUNC;
    if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        res->func_types = (ParamTypeList){
            .is_variadic = false,
            .param_list =
                {
                    .len = 0,
                    .decls = NULL,
                },
        };
        ParserState_accept_it(s);
    } else {
        if (!parse_param_type_list(s, &res->func_types)) {
            return false;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            ParamTypeList_free(&res->func_types);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_suffix(ParserState* s, AbsArrOrFuncSuffix* res) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        res->kind = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = false;
        ParserState_accept_it(s);
        return true;
    } else if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        res->kind = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = true;
        ParserState_accept_it(s);
        res->assign = NULL;
        if (!ParserState_accept(s, TOKEN_RINDEX)) {
            return false;
        }
        return true;
    }

    res->kind = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN;
    res->is_static = false;
    if (ParserState_curr_kind(s) == TOKEN_STATIC) {
        ParserState_accept_it(s);
        res->is_static = true;
    }

    if (is_type_qual(ParserState_curr_kind(s))) {
        if (!parse_type_qual_list(s, &res->type_quals)) {
            return false;
        }

        if (ParserState_curr_kind(s) == TOKEN_STATIC) {
            if (res->is_static) {
                ParserErr_set(s->err,
                              PARSER_ERR_ARR_DOUBLE_STATIC,
                              ParserState_curr_loc(s));
                AbsArrOrFuncSuffix_free(res);
                return false;
            } else {
                ParserState_accept_it(s);
                res->is_static = true;
            }
        }
    }

    if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        if (res->is_static) {
            ParserErr_set(s->err,
                          PARSER_ERR_ARR_STATIC_NO_LEN,
                          ParserState_curr_loc(s));
            AbsArrOrFuncSuffix_free(res);
            return false;
        }
        res->assign = NULL;
        ParserState_accept_it(s);
    } else {
        res->assign = parse_assign_expr(s);
        if (!(res->assign && ParserState_accept(s, TOKEN_RINDEX))) {
            AbsArrOrFuncSuffix_free(res);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_or_func_suffix(ParserState* s,
                                         AbsArrOrFuncSuffix* res) {
    assert(res);
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));

    switch (ParserState_curr_kind(s)) {
        case TOKEN_LBRACKET:
            return parse_abs_func_suffix(s, res);
        case TOKEN_LINDEX:
            return parse_abs_arr_suffix(s, res);
        default:
            UNREACHABLE();
    }
}

bool parse_abs_arr_or_func_suffixes(ParserState* s, DirectAbsDeclarator* res) {
    res->following_suffixes = NULL;
    res->len = 0;
    size_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->following_suffixes,
                            &alloc_len,
                            sizeof *res->following_suffixes);
        }

        if (!parse_abs_arr_or_func_suffix(s,
                                          &res->following_suffixes[res->len])) {
            DirectAbsDeclarator_free(res);
            return false;
        }

        ++res->len;
    }

    res->following_suffixes = mycc_realloc(res->following_suffixes,
                                           sizeof *res->following_suffixes
                                               * res->len);
    return true;
}

struct DirectAbsDeclarator* parse_direct_abs_declarator(ParserState* s) {
    struct DirectAbsDeclarator* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    const TokenKind next_kind = ParserState_next_token_kind(s);
    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        && (next_kind == TOKEN_LBRACKET || next_kind == TOKEN_LINDEX
            || next_kind == TOKEN_ASTERISK)) {
        ParserState_accept_it(s);
        res->bracket_decl = parse_abs_declarator(s);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            AbsDeclarator_free(res->bracket_decl);
            mycc_free(res);
            return NULL;
        }
    } else {
        res->bracket_decl = NULL;
    }

    if (!parse_abs_arr_or_func_suffixes(s, res)) {
        return NULL;
    }
    return res;
}

static void AbsArrOrFuncSuffix_free(AbsArrOrFuncSuffix* s) {
    switch (s->kind) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            if (s->assign) {
                AssignExpr_free(s->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            ParamTypeList_free(&s->func_types);
            break;
    }
}

static void DirectAbsDeclarator_free_children(struct DirectAbsDeclarator* d) {
    if (d->bracket_decl) {
        AbsDeclarator_free(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        AbsArrOrFuncSuffix_free(&d->following_suffixes[i]);
    }
    mycc_free(d->following_suffixes);
}

void DirectAbsDeclarator_free(struct DirectAbsDeclarator* d) {
    DirectAbsDeclarator_free_children(d);
    mycc_free(d);
}

