#include "frontend/ast/declaration/direct_declarator.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static void free_arr_suffix(struct arr_suffix* s) {
    if (s->arr_len) {
        free_assign_expr(s->arr_len);
    }
}

static bool parse_arr_suffix(struct parser_state* s,
                             struct arr_or_func_suffix* res) {
    assert(s->it->kind == TOKEN_LINDEX);

    res->kind = ARR_OR_FUNC_ARRAY;
    struct arr_suffix* suffix = &res->arr_suffix;
    *suffix = (struct arr_suffix){
        .is_static = false,
        .type_quals = create_type_quals(),
        .is_asterisk = false,
        .arr_len = NULL,
    };
    accept_it(s);
    if (s->it->kind == TOKEN_ASTERISK) {
        accept_it(s);
        suffix->is_asterisk = true;
        if (!accept(s, TOKEN_RINDEX)) {
            return false;
        }
        return true;
    } else if (s->it->kind == TOKEN_RINDEX) {
        accept_it(s);
        return true;
    }

    if (s->it->kind == TOKEN_STATIC) {
        accept_it(s);
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
            accept_it(s);
            suffix->is_asterisk = true;
            if (!accept(s, TOKEN_RINDEX)) {
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
        accept_it(s);
    }

    if (s->it->kind == TOKEN_RINDEX) {
        if (suffix->is_static) {
            set_parser_err(s->err, PARSER_ERR_ARR_STATIC_NO_LEN, s->it->loc);
            free_arr_suffix(suffix);
            return false;
        }
        accept_it(s);
    } else {
        suffix->arr_len = parse_assign_expr(s);
        if (!(suffix->arr_len && accept(s, TOKEN_RINDEX))) {
            free_arr_suffix(suffix);
            return false;
        }
    }

    return true;
}

static bool parse_func_suffix(struct parser_state* s,
                              struct arr_or_func_suffix* res) {
    assert(s->it->kind == TOKEN_LBRACKET);

    accept_it(s);
    if (s->it->kind == TOKEN_IDENTIFIER && !is_typedef_name(s, &s->it->spelling)) {
        res->kind = ARR_OR_FUNC_FUN_OLD_PARAMS;
        if (!parse_identifier_list(s, &res->fun_params)) {
            return false;
        }

        if (!accept(s, TOKEN_RBRACKET)) {
            free_identifier_list(&res->fun_params);
            return false;
        }
    } else if (s->it->kind == TOKEN_RBRACKET) {
        accept_it(s);
        res->kind = ARR_OR_FUNC_FUN_EMPTY;
    } else {
        res->kind = ARR_OR_FUNC_FUN_PARAMS;
        if (!parse_param_type_list(s, &res->fun_types)) {
            return false;
        }

        if (!accept(s, TOKEN_RBRACKET)) {
            free_param_type_list(&res->fun_types);
            return false;
        }
    }
    return true;
}

static bool parse_arr_or_func_suffix(struct parser_state* s,
                                     struct arr_or_func_suffix* res) {
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

bool parse_arr_or_func_suffixes(struct parser_state* s,
                                struct direct_declarator* res) {
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

    res->suffixes = mycc_realloc(res->suffixes, sizeof *res->suffixes * res->len);

    return true;
}

static struct direct_declarator* parse_direct_declarator_base(
    struct parser_state* s,
    struct declarator* (*parse_func)(struct parser_state*),
    bool (*identifier_handler)(struct parser_state*, const struct token*)) {
    struct direct_declarator* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->kind == TOKEN_LBRACKET) {
        accept_it(s);
        res->is_id = false;
        res->bracket_decl = parse_func(s);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }

        if (!accept(s, TOKEN_RBRACKET)) {
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
        const struct str spelling = token_take_spelling(s->it);
        struct source_loc loc = s->it->loc;
        accept_it(s);
        res->id = create_identifier(&spelling, loc);
    } else {
        mycc_free(res);
        enum token_kind expected[] = {TOKEN_LBRACKET, TOKEN_IDENTIFIER};
        expected_tokens_error(s, expected, ARR_LEN(expected));
        return NULL;
    }

    if (!parse_arr_or_func_suffixes(s, res)) {
        return false;
    }

    return res;
}

static bool empty_id_handler(struct parser_state* s,
                             const struct token* token) {
    UNUSED(s);
    UNUSED(token);
    return true;
}

struct direct_declarator* parse_direct_declarator(struct parser_state* s) {
    return parse_direct_declarator_base(s, parse_declarator, empty_id_handler);
}

struct direct_declarator* parse_direct_declarator_typedef(
    struct parser_state* s) {
    return parse_direct_declarator_base(s,
                                        parse_declarator_typedef,
                                        register_typedef_name);
}

static void free_children(struct direct_declarator* d) {
    if (d->is_id) {
        free_identifier(d->id);
    } else {
        free_declarator(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        struct arr_or_func_suffix* item = &d->suffixes[i];
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

void free_direct_declarator(struct direct_declarator* d) {
    free_children(d);
    mycc_free(d);
}

