#include "frontend/ast/declaration/direct_abs_declarator.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

static void free_abs_arr_or_func_suffix(struct abs_arr_or_func_suffix* s);

static bool parse_abs_func_suffix(struct parser_state* s,
                                  struct abs_arr_or_func_suffix* res) {
    assert(s->it->type == LBRACKET);
    accept_it(s);
    res->type = ABS_ARR_OR_FUNC_SUFFIX_FUNC;
    if (s->it->type == RBRACKET) {
        res->func_types = (struct param_type_list){
            .is_variadic = false,
            .param_list = NULL,
        };
        accept_it(s);
    } else {
        if (!parse_param_type_list(s, &res->func_types)) {
            return false;
        }

        if (!accept(s, RBRACKET)) {
            free_param_type_list(&res->func_types);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_suffix(struct parser_state* s,
                                 struct abs_arr_or_func_suffix* res) {
    assert(s->it->type == LINDEX);
    accept_it(s);
    if (s->it->type == RINDEX) {
        res->type = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = false;
        accept_it(s);
        return true;
    } else if (s->it->type == ASTERISK) {
        res->type = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY;
        res->has_asterisk = true;
        accept_it(s);
        res->assign = NULL;
        if (!accept(s, RINDEX)) {
            return false;
        }
        return true;
    }

    res->type = ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN;
    res->is_static = false;
    if (s->it->type == STATIC) {
        accept_it(s);
        res->is_static = true;
    }

    if (is_type_qual(s->it->type)) {
        if (!parse_type_qual_list(s, &res->type_quals)) {
            return false;
        }

        if (s->it->type == STATIC) {
            if (res->is_static) {
                set_parser_err(s->err,
                               PARSER_ERR_ARR_DOUBLE_STATIC,
                               s->it->loc);
                free_abs_arr_or_func_suffix(res);
                return false;
            } else {
                accept_it(s);
                res->is_static = true;
            }
        }
    }

    if (s->it->type == RINDEX) {
        if (res->is_static) {
            set_parser_err(s->err, PARSER_ERR_ARR_STATIC_NO_LEN, s->it->loc);
            free_abs_arr_or_func_suffix(res);
            return false;
        }
        res->assign = NULL;
        accept_it(s);
    } else {
        res->assign = parse_assign_expr(s);
        if (!(res->assign && accept(s, RINDEX))) {
            free_abs_arr_or_func_suffix(res);
            return false;
        }
    }
    return true;
}

static bool parse_abs_arr_or_func_suffix(struct parser_state* s,
                                         struct abs_arr_or_func_suffix* res) {
    assert(res);
    assert(s->it->type == LBRACKET || s->it->type == LINDEX);
    res->info = create_ast_node_info(s->it->loc);

    switch (s->it->type) {
        case LBRACKET:
            return parse_abs_func_suffix(s, res);
        case LINDEX:
            return parse_abs_arr_suffix(s, res);
        default:
            UNREACHABLE();
    }
}

bool parse_abs_arr_or_func_suffixes(struct parser_state* s,
                                    struct direct_abs_declarator* res) {
    res->following_suffixes = NULL;
    res->len = 0;
    size_t alloc_len = res->len;
    while (s->it->type == LBRACKET || s->it->type == LINDEX) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->following_suffixes,
                       &alloc_len,
                       sizeof *res->following_suffixes);
        }

        if (!parse_abs_arr_or_func_suffix(s,
                                          &res->following_suffixes[res->len])) {
            free_direct_abs_declarator(res);
            return false;
        }

        ++res->len;
    }

    res->following_suffixes = mycc_realloc(res->following_suffixes,
                                       sizeof *res->following_suffixes
                                           * res->len);
    return true;
}

struct direct_abs_declarator* parse_direct_abs_declarator(
    struct parser_state* s) {
    struct direct_abs_declarator* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->type == LBRACKET
        && (s->it[1].type == LBRACKET || s->it[1].type == LINDEX
            || s->it[1].type == ASTERISK)) {
        accept_it(s);
        res->bracket_decl = parse_abs_declarator(s);
        if (!res->bracket_decl) {
            mycc_free(res);
            return NULL;
        }

        if (!accept(s, RBRACKET)) {
            free_abs_declarator(res->bracket_decl);
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

static void free_abs_arr_or_func_suffix(struct abs_arr_or_func_suffix* s) {
    switch (s->type) {
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_EMPTY:
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_ARRAY_DYN:
            if (s->assign) {
                free_assign_expr(s->assign);
            }
            break;
        case ABS_ARR_OR_FUNC_SUFFIX_FUNC:
            free_param_type_list(&s->func_types);
            break;
    }
}

static void free_children(struct direct_abs_declarator* d) {
    if (d->bracket_decl) {
        free_abs_declarator(d->bracket_decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        free_abs_arr_or_func_suffix(&d->following_suffixes[i]);
    }
    mycc_free(d->following_suffixes);
}

void free_direct_abs_declarator(struct direct_abs_declarator* d) {
    free_children(d);
    mycc_free(d);
}

