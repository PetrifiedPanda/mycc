#include "frontend/ast/declaration/DirectDeclarator.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

#include "frontend/ast/Identifier.h"
#include "frontend/ast/AssignExpr.h"

#include "frontend/ast/declaration/Declarator.h"

static void ArrSuffix_free(ArrSuffix* s) {
    if (s->arr_len) {
        AssignExpr_free(s->arr_len);
    }
}

static bool parse_arr_suffix(ParserState* s, ArrOrFuncSuffix* res) {
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX);

    res->kind = ARR_OR_FUNC_ARRAY;
    ArrSuffix* suffix = &res->arr_suffix;
    *suffix = (ArrSuffix){
        .is_static = false,
        .type_quals = TypeQuals_create(),
        .is_asterisk = false,
        .arr_len = NULL,
    };
    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
        ParserState_accept_it(s);
        suffix->is_asterisk = true;
        if (!ParserState_accept(s, TOKEN_RINDEX)) {
            return false;
        }
        return true;
    } else if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        ParserState_accept_it(s);
        return true;
    }

    if (ParserState_curr_kind(s) == TOKEN_STATIC) {
        ParserState_accept_it(s);
        suffix->is_static = true;
    }

    if (is_type_qual(ParserState_curr_kind(s))) {
        if (!parse_type_qual_list(s, &suffix->type_quals)) {
            return false;
        }

        if (ParserState_curr_kind(s) == TOKEN_ASTERISK) {
            if (suffix->is_static) {
                ParserErr_set(s->err, PARSER_ERR_ARR_STATIC_ASTERISK, s->it);
                ArrSuffix_free(suffix);
                return false;
            }
            ParserState_accept_it(s);
            suffix->is_asterisk = true;
            if (!ParserState_accept(s, TOKEN_RINDEX)) {
                ArrSuffix_free(suffix);
                return false;
            }
            return true;
        }
    }

    if (ParserState_curr_kind(s) == TOKEN_STATIC) {
        if (suffix->is_static) {
            ParserErr_set(s->err, PARSER_ERR_ARR_DOUBLE_STATIC, s->it);
            ArrSuffix_free(suffix);
            return false;
        }
        suffix->is_static = true;
        ParserState_accept_it(s);
    }

    if (ParserState_curr_kind(s) == TOKEN_RINDEX) {
        if (suffix->is_static) {
            ParserErr_set(s->err, PARSER_ERR_ARR_STATIC_NO_LEN, s->it);
            ArrSuffix_free(suffix);
            return false;
        }
        ParserState_accept_it(s);
    } else {
        suffix->arr_len = parse_assign_expr(s);
        if (!(suffix->arr_len && ParserState_accept(s, TOKEN_RINDEX))) {
            ArrSuffix_free(suffix);
            return false;
        }
    }

    return true;
}

static bool parse_func_suffix(ParserState* s, ArrOrFuncSuffix* res) {
    assert(ParserState_curr_kind(s) == TOKEN_LBRACKET);

    ParserState_accept_it(s);
    if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER
        && !ParserState_is_typedef(s, ParserState_curr_spell(s))) {
        res->kind = ARR_OR_FUNC_FUN_OLD_PARAMS;
        if (!parse_identifier_list(s, &res->fun_params)) {
            return false;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            IdentifierList_free(&res->fun_params);
            return false;
        }
    } else if (ParserState_curr_kind(s) == TOKEN_RBRACKET) {
        ParserState_accept_it(s);
        res->kind = ARR_OR_FUNC_FUN_EMPTY;
    } else {
        res->kind = ARR_OR_FUNC_FUN_PARAMS;
        if (!parse_param_type_list(s, &res->fun_types)) {
            return false;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            ParamTypeList_free(&res->fun_types);
            return false;
        }
    }
    return true;
}

static bool parse_arr_or_func_suffix(ParserState* s, ArrOrFuncSuffix* res) {
    assert(res);
    assert(ParserState_curr_kind(s) == TOKEN_LINDEX
           || ParserState_curr_kind(s) == TOKEN_LBRACKET);
    res->info = AstNodeInfo_create(s->it);
    switch (ParserState_curr_kind(s)) {
        case TOKEN_LINDEX:
            return parse_arr_suffix(s, res);

        case TOKEN_LBRACKET:
            return parse_func_suffix(s, res);

        default:
            UNREACHABLE();
    }
}

bool parse_arr_or_func_suffixes(ParserState* s, DirectDeclarator* res) {
    res->suffixes = NULL;
    res->len = 0;
    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_LBRACKET
           || ParserState_curr_kind(s) == TOKEN_LINDEX) {
        if (alloc_len == res->len) {
            mycc_grow_alloc((void**)&res->suffixes,
                            &alloc_len,
                            sizeof *res->suffixes);
        }

        if (!parse_arr_or_func_suffix(s, &res->suffixes[res->len])) {
            DirectDeclarator_free(res);
            return false;
        }

        ++res->len;
    }

    res->suffixes = mycc_realloc(res->suffixes,
                                 sizeof *res->suffixes * res->len);

    return true;
}

static DirectDeclarator* parse_direct_declarator_base(
    ParserState* s,
    Declarator* (*parse_func)(ParserState*),
    bool (*identifier_handler)(ParserState*, const StrBuf*, uint32_t)) {
    DirectDeclarator* res = mycc_alloc(sizeof *res);
    res->info = AstNodeInfo_create(s->it);
    if (ParserState_curr_kind(s) == TOKEN_LBRACKET) {
        ParserState_accept_it(s);
        res->is_id = false;
        res->bracket_decl = parse_func(s);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            Declarator_free(res->bracket_decl);
            mycc_free(res);
            return NULL;
        }
    } else if (ParserState_curr_kind(s) == TOKEN_IDENTIFIER) {
        res->is_id = true;
        if (!identifier_handler(s, ParserState_curr_spell_buf(s), s->it)) {
            mycc_free(res);
            return NULL;
        }
        const uint32_t idx = s->it;
        ParserState_accept_it(s);
        res->id = Identifier_create(idx);
    } else {
        mycc_free(res);
        static const TokenKind expected[] = {
            TOKEN_LBRACKET,
            TOKEN_IDENTIFIER,
        };
        expected_tokens_error(s, expected, ARR_LEN(expected));
        return NULL;
    }

    if (!parse_arr_or_func_suffixes(s, res)) {
        return NULL;
    }

    return res;
}

static bool empty_id_handler(ParserState* s,
                             const StrBuf* spell,
                             uint32_t idx) {
    UNUSED(s);
    UNUSED(spell);
    UNUSED(idx);
    return true;
}

DirectDeclarator* parse_direct_declarator(ParserState* s) {
    return parse_direct_declarator_base(s, parse_declarator, empty_id_handler);
}

DirectDeclarator* parse_direct_declarator_typedef(ParserState* s) {
    return parse_direct_declarator_base(s,
                                        parse_declarator_typedef,
                                        ParserState_register_typedef);
}

static void DirectDeclarator_free_children(DirectDeclarator* d) {
    if (d->is_id) {
        Identifier_free(d->id);
    } else {
        Declarator_free(d->bracket_decl);
    }

    for (uint32_t i = 0; i < d->len; ++i) {
        ArrOrFuncSuffix* item = &d->suffixes[i];
        switch (item->kind) {
            case ARR_OR_FUNC_ARRAY:
                ArrSuffix_free(&item->arr_suffix);
                break;
            case ARR_OR_FUNC_FUN_PARAMS:
                ParamTypeList_free(&item->fun_types);
                break;
            case ARR_OR_FUNC_FUN_OLD_PARAMS:
                IdentifierList_free(&item->fun_params);
                break;
            case ARR_OR_FUNC_FUN_EMPTY:
                break;
            default:
                UNREACHABLE();
        }
    }
    mycc_free(d->suffixes);
}

void DirectDeclarator_free(DirectDeclarator* d) {
    DirectDeclarator_free_children(d);
    mycc_free(d);
}

