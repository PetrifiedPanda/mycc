#include "ast/direct_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "error.h"
#include "util.h"

#include "parser/parser_util.h"

static void free_arr_suffix(struct arr_suffix* s) {
    if (s->arr_len) {
        free_assign_expr(s->arr_len);
    }
}

static bool parse_arr_or_func_suffix(struct parser_state* s, struct arr_or_func_suffix* res) {
    assert(res);
    assert(s->it->type == LINDEX || s->it->type == LBRACKET);

    switch (s->it->type) {
        case LINDEX: {
            res->type = ARR_OR_FUNC_ARRAY;
            struct arr_suffix* suffix = &res->arr_suffix;
            *suffix = (struct arr_suffix) {
                .is_static = false,
                .type_quals = create_type_quals(),
                .is_asterisk = false,
                .arr_len = NULL
            };
            accept_it(s);
            if (s->it->type == ASTERISK) {
                accept_it(s);
                suffix->is_asterisk = true;
                if (!accept(s, RINDEX)) {
                    return false;
                }
                return true;
            } else if (s->it->type == RINDEX) {
                accept_it(s);
                return true;
            }

            if (s->it->type == STATIC) {
                accept_it(s);
                suffix->is_static = true;
            }

            if (is_type_qual(s->it->type)) {
                suffix->type_quals = parse_type_qual_list(s);
                if (!is_valid_type_quals(&suffix->type_quals)) {
                    return false;
                }

                if (s->it->type == ASTERISK) {
                    if (suffix->is_static) {
                        free_arr_suffix(suffix);
                        set_error_file(ERR_PARSER, s->it->file, s->it->source_loc, "Asterisk cannot be used with static");
                        return false;
                    }
                    suffix->is_asterisk = true;
                    if (!accept(s, RINDEX)) {
                        free_arr_suffix(suffix);
                        return false;
                    }
                    return true;
                }
            }

            if (s->it->type == STATIC) {
                if (suffix->is_static) {
                    free_arr_suffix(suffix);
                    // TODO: maybe turn this into a warning
                    set_error_file(ERR_PARSER, s->it->file, s->it->source_loc, "Expected only one use of static");
                    return false;
                }
                suffix->is_static = true;
                accept_it(s);
            }

            if (s->it->type == RINDEX) {
                if (suffix->is_static) {
                    free_arr_suffix(suffix);
                    set_error_file(ERR_PARSER, s->it->file, s->it->source_loc, "Expected array size after use of static");
                    return false;
                }
                accept_it(s);
            } else {
                suffix->arr_len = parse_assign_expr(s);
                if (!(suffix->arr_len && accept(s, RINDEX))) {
                    free_arr_suffix(suffix);
                    return false;
                }

            }

            return true;
        }

        case LBRACKET: {
            accept_it(s);
            if (s->it->type == IDENTIFIER && !is_typedef_name(s, s->it->spelling)) {
                res->type = ARR_OR_FUNC_FUN_PARAMS;
                res->fun_params = parse_identifier_list(s);
                if (res->fun_params.len == 0) {
                    return false;
                }

                if (!accept(s, RBRACKET)) {
                    free_identifier_list(&res->fun_params);
                    return false;
                }
            } else if (s->it->type == RBRACKET) {
                accept_it(s);
                res->type = ARR_OR_FUNC_FUN_EMPTY;
            } else {
                res->type = ARR_OR_FUNC_FUN_TYPES;
                res->fun_types = parse_param_type_list(s);
                if (res->fun_types.param_list == NULL) {
                    return false;
                }

                if (!accept(s, RBRACKET)) {
                    free_param_type_list(&res->fun_types);
                    return false;
                }
            }
            return true;
        }

        default: // UNREACHABLE
            assert(false);
    }
    return false;
}

static struct direct_declarator* parse_direct_declarator_base(struct parser_state* s, struct declarator* (*parse_func)(struct parser_state*), bool (*identifier_handler)(struct parser_state*, const struct token*)) {
    struct direct_declarator* res = xmalloc(sizeof(struct direct_declarator));
    if (s->it->type == LBRACKET) {
        accept_it(s);
        res->is_id = false;
        res->decl = parse_func(s);
        if (!res->decl) {
            free(res);
            return NULL;
        }

        if (!accept(s, RBRACKET)) {
            free_declarator(res->decl);
            free(res);
            return NULL;
        }
    } else if (s->it->type == IDENTIFIER) {
        res->is_id = true;
        if (!identifier_handler(s, s->it)) {
            return NULL;
        } 
        char* spelling = take_spelling(s->it);
        accept_it(s);
        res->id = create_identifier(spelling);
    } else {
        free(res);
        enum token_type expected[] = {
            LBRACKET,
            IDENTIFIER
        };
        expected_tokens_error(expected, sizeof expected / sizeof(enum token_type), s->it);
        return NULL;
    }

    res->suffixes = NULL;
    size_t alloc_len = res->len = 0;
    while (s->it->type == LBRACKET || s->it->type == LINDEX) {
        if (alloc_len == res->len) {
            grow_alloc((void**)&res->suffixes, &alloc_len, sizeof(struct arr_or_func_suffix));
        }

        if (!parse_arr_or_func_suffix(s, &res->suffixes[res->len])) {
            free_direct_declarator(res);
            return false;
        }

        ++res->len;
    }

    res->suffixes = xrealloc(res->suffixes, sizeof(struct arr_or_func_suffix) * res->len);

    return res;   
}

static bool empty_id_handler(struct parser_state* s, const struct token* token) {
    UNUSED(s);
    UNUSED(token);
    return true;
}

struct direct_declarator* parse_direct_declarator(struct parser_state* s) {
    return parse_direct_declarator_base(s, parse_declarator, empty_id_handler);
}

struct direct_declarator* parse_direct_declarator_typedef(struct parser_state* s) {
    return parse_direct_declarator_base(s, parse_declarator_typedef, register_typedef_name);
}

static void free_children(struct direct_declarator* d) {
    if (d->is_id) {
        free_identifier(d->id);
    } else {
        free_declarator(d->decl);
    }

    for (size_t i = 0; i < d->len; ++i) {
        struct arr_or_func_suffix* item = &d->suffixes[i];
        switch (item->type) {
            case ARR_OR_FUNC_ARRAY:
                free_arr_suffix(&item->arr_suffix);
                break;
            case ARR_OR_FUNC_FUN_TYPES:
                free_param_type_list(&item->fun_types);
                break;
            case ARR_OR_FUNC_FUN_PARAMS:
                free_identifier_list(&item->fun_params);
                break;
            case ARR_OR_FUNC_FUN_EMPTY:
                break;
        }
    }
    free(d->suffixes);
}

void free_direct_declarator(struct direct_declarator* d) {
    free_children(d);
    free(d);
}

