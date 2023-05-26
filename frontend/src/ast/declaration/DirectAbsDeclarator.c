#include "frontend/ast/declaration/DirectAbsDeclarator.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static void free_abs_arr_or_func_suffix(AbsArrOrFuncSuffix* s);

static bool parse_abs_func_suffix(ParserState* s, AbsArrOrFuncSuffix* res) {
    assert(s->it->kind == TOKEN_LBRACKET);
    parser_accept_it(s);
    res->kind = ABS_ARR_OR_FUNC_SUFFIX_FUNC;
    if (s->it->kind == TOKEN_RBRACKET) {
        res->func_types = (ParamTypeList){
            .is_variadic = false,
            .param_list =
                {
                    .len = 0,
                    .decls = NULL,
                },
        };
        parser_accept_it(s);
    } else {
        if (!parse_param_type_list(s, &res->func_types)) {
            return false;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
            ParamTypeList_free(&res->func_types);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_suffix(ParserState* s, AbsArrOrFuncSuffix* res) {
    assert(s->it->kind == TOKEN_LINDEX);
    parser_accept_it(s);
    if (s->it->kind == TOKEN_RINDEX) {
        res->kind = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = false;
        parser_accept_it(s);
        return true;
    } else if (s->it->kind == TOKEN_ASTERISK) {
        res->kind = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = true;
        parser_accept_it(s);
        res->assign = NULL;
        if (!parser_accept(s, TOKEN_RINDEX)) {
            return false;
        }
        return true;
    }

    res->kind = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN;
    res->is_static = false;
    if (s->it->kind == TOKEN_STATIC) {
        parser_accept_it(s);
        res->is_static = true;
    }

    if (is_type_qual(s->it->kind)) {
        if (!parse_type_qual_list(s, &res->type_quals)) {
            return false;
        }

        if (s->it->kind == TOKEN_STATIC) {
            if (res->is_static) {
                ParserErr_set(s->err,
                               PARSER_ERR_ARR_DOUBLE_STATIC,
                               s->it->loc);
                free_abs_arr_or_func_suffix(res);
                return false;
            } else {
                parser_accept_it(s);
                res->is_static = true;
            }
        }
    }

    if (s->it->kind == TOKEN_RINDEX) {
        if (res->is_static) {
            ParserErr_set(s->err, PARSER_ERR_ARR_STATIC_NO_LEN, s->it->loc);
            free_abs_arr_or_func_suffix(res);
            return false;
        }
        res->assign = NULL;
        parser_accept_it(s);
    } else {
        res->assign = parse_assign_expr(s);
        if (!(res->assign && parser_accept(s, TOKEN_RINDEX))) {
            free_abs_arr_or_func_suffix(res);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_or_func_suffix(ParserState* s, AbsArrOrFuncSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LBRACKET || s->it->kind == TOKEN_LINDEX);
    res->info = AstNodeInfo_create(s->it->loc);

    switch (s->it->kind) {
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
    while (s->it->kind == TOKEN_LBRACKET || s->it->kind == TOKEN_LINDEX) {
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
    res->info = AstNodeInfo_create(s->it->loc);
    if (s->it->kind == TOKEN_LBRACKET
        && (s->it[1].kind == TOKEN_LBRACKET || s->it[1].kind == TOKEN_LINDEX
            || s->it[1].kind == TOKEN_ASTERISK)) {
        parser_accept_it(s);
        res->bracket_decl = parse_abs_declarator(s);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
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

static void free_abs_arr_or_func_suffix(AbsArrOrFuncSuffix* s) {
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

static void free_direct_abs_declarator_children(
    struct DirectAbsDeclarator* d) {
    if (d->bracket_decl) {
        AbsDeclarator_free(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        free_abs_arr_or_func_suffix(&d->following_suffixes[i]);
    }
    mycc_free(d->following_suffixes);
}

void DirectAbsDeclarator_free(struct DirectAbsDeclarator* d) {
    free_direct_abs_declarator_children(d);
    mycc_free(d);
}

