#include "frontend/ast/declaration/DirectDeclarator.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static void free_arr_suffix(ArrSuffix* s) {
    if (s->arr_len) {
        free_assign_expr(s->arr_len);
    }
}

static bool parse_arr_suffix(ParserState* s, ArrOrFuncSuffix* res) {
    assert(s->it->kind == TOKEN_LINDEX);

    res->kind = ARR_OR_FUNC_ARRAY;
    ArrSuffix* suffix = &res->arr_suffix;
    *suffix = (ArrSuffix){
        .is_static = false,
        .type_quals = create_type_quals(),
        .is_asterisk = false,
        .arr_len = NULL,
    };
    parser_accept_it(s);
    if (s->it->kind == TOKEN_ASTERISK) {
        parser_accept_it(s);
        suffix->is_asterisk = true;
        if (!parser_accept(s, TOKEN_RINDEX)) {
            return false;
        }
        return true;
    } else if (s->it->kind == TOKEN_RINDEX) {
        parser_accept_it(s);
        return true;
    }

    if (s->it->kind == TOKEN_STATIC) {
        parser_accept_it(s);
        suffix->is_static = true;
    }

    if (is_type_qual(s->it->kind)) {
        if (!parse_type_qual_list(s, &suffix->type_quals)) {
            return false;
        }

        if (s->it->kind == TOKEN_ASTERISK) {
            if (suffix->is_static) {
                set_parser_err(s->err,
                               PARSER_ERR_ARR_STATIC_ASTERISK,
                               s->it->loc);
                free_arr_suffix(suffix);
                return false;
            }
            parser_accept_it(s);
            suffix->is_asterisk = true;
            if (!parser_accept(s, TOKEN_RINDEX)) {
                free_arr_suffix(suffix);
                return false;
            }
            return true;
        }
    }

    if (s->it->kind == TOKEN_STATIC) {
        if (suffix->is_static) {
            set_parser_err(s->err, PARSER_ERR_ARR_DOUBLE_STATIC, s->it->loc);
            free_arr_suffix(suffix);
            return false;
        }
        suffix->is_static = true;
        parser_accept_it(s);
    }

    if (s->it->kind == TOKEN_RINDEX) {
        if (suffix->is_static) {
            set_parser_err(s->err, PARSER_ERR_ARR_STATIC_NO_LEN, s->it->loc);
            free_arr_suffix(suffix);
            return false;
        }
        parser_accept_it(s);
    } else {
        suffix->arr_len = parse_assign_expr(s);
        if (!(suffix->arr_len && parser_accept(s, TOKEN_RINDEX))) {
            free_arr_suffix(suffix);
            return false;
        }
    }

    return true;
}

static bool parse_func_suffix(ParserState* s, ArrOrFuncSuffix* res) {
    assert(s->it->kind == TOKEN_LBRACKET);

    parser_accept_it(s);
    if (s->it->kind == TOKEN_IDENTIFIER
        && !parser_is_typedef_name(s, &s->it->spelling)) {
        res->kind = ARR_OR_FUNC_FUN_OLD_PARAMS;
        if (!parse_identifier_list(s, &res->fun_params)) {
            return false;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
            free_identifier_list(&res->fun_params);
            return false;
        }
    } else if (s->it->kind == TOKEN_RBRACKET) {
        parser_accept_it(s);
        res->kind = ARR_OR_FUNC_FUN_EMPTY;
    } else {
        res->kind = ARR_OR_FUNC_FUN_PARAMS;
        if (!parse_param_type_list(s, &res->fun_types)) {
            return false;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
            free_param_type_list(&res->fun_types);
            return false;
        }
    }
    return true;
}

static bool parse_arr_or_func_suffix(ParserState* s, ArrOrFuncSuffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LINDEX || s->it->kind == TOKEN_LBRACKET);
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->kind) {
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
    size_t alloc_len = res->len;
    while (s->it->kind == TOKEN_LBRACKET || s->it->kind == TOKEN_LINDEX) {
        if (alloc_len == res->len) {
            mycc_grow_alloc((void**)&res->suffixes,
                            &alloc_len,
                            sizeof *res->suffixes);
        }

        if (!parse_arr_or_func_suffix(s, &res->suffixes[res->len])) {
            free_direct_declarator(res);
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
    bool (*identifier_handler)(ParserState*, const Token*)) {
    DirectDeclarator* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->kind == TOKEN_LBRACKET) {
        parser_accept_it(s);
        res->is_id = false;
        res->bracket_decl = parse_func(s);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
            free_declarator(res->bracket_decl);
            mycc_free(res);
            return NULL;
        }
    } else if (s->it->kind == TOKEN_IDENTIFIER) {
        res->is_id = true;
        if (!identifier_handler(s, s->it)) {
            mycc_free(res);
            return NULL;
        }
        const Str spelling = token_take_spelling(s->it);
        const SourceLoc loc = s->it->loc;
        parser_accept_it(s);
        res->id = create_identifier(&spelling, loc);
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
        return false;
    }

    return res;
}

static bool empty_id_handler(ParserState* s, const Token* token) {
    UNUSED(s);
    UNUSED(token);
    return true;
}

DirectDeclarator* parse_direct_declarator(ParserState* s) {
    return parse_direct_declarator_base(s, parse_declarator, empty_id_handler);
}

DirectDeclarator* parse_direct_declarator_typedef(
    ParserState* s) {
    return parse_direct_declarator_base(s,
                                        parse_declarator_typedef,
                                        parser_register_typedef_name);
}

static void free_direct_declarator_children(DirectDeclarator* d) {
    if (d->is_id) {
        free_identifier(d->id);
    } else {
        free_declarator(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        ArrOrFuncSuffix* item = &d->suffixes[i];
        switch (item->kind) {
            case ARR_OR_FUNC_ARRAY:
                free_arr_suffix(&item->arr_suffix);
                break;
            case ARR_OR_FUNC_FUN_PARAMS:
                free_param_type_list(&item->fun_types);
                break;
            case ARR_OR_FUNC_FUN_OLD_PARAMS:
                free_identifier_list(&item->fun_params);
                break;
            case ARR_OR_FUNC_FUN_EMPTY:
                break;
            default:
                UNREACHABLE();
        }
    }
    mycc_free(d->suffixes);
}

void free_direct_declarator(DirectDeclarator* d) {
    free_direct_declarator_children(d);
    mycc_free(d);
}

