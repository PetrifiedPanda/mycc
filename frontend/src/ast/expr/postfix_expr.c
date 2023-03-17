#include "frontend/ast/expr/postfix_expr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool is_posfix_op(enum token_kind t) {
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

static bool parse_postfix_arr_suffix(struct parser_state* s,
                                     struct postfix_suffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LINDEX);

    accept_it(s);
    struct expr* expr = parse_expr(s);
    if (!expr) {
        return false;
    }
    if (!accept(s, TOKEN_RINDEX)) {
        free_expr(expr);
        return false;
    }

    res->kind = POSTFIX_INDEX;
    res->index_expr = expr;
    return true;
}

static bool parse_postfix_func_suffix(struct parser_state* s,
                                      struct postfix_suffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_LBRACKET);

    accept_it(s);
    struct arg_expr_list arg_expr_list = {
        .assign_exprs = NULL,
        .len = 0,
    };
    if (s->it->kind != TOKEN_RBRACKET) {
        if (!parse_arg_expr_list(s, &arg_expr_list)) {
            return false;
        }
    }
    if (!accept(s, TOKEN_RBRACKET)) {
        free_arg_expr_list(&arg_expr_list);
        return false;
    }
    res->kind = POSTFIX_BRACKET;
    res->bracket_list = arg_expr_list;
    return true;
}

static bool parse_postfix_access_suffix(struct parser_state* s,
                                        struct postfix_suffix* res) {
    assert(res);
    assert(s->it->kind == TOKEN_DOT || s->it->kind == TOKEN_PTR_OP);
    enum postfix_suffix_kind kind = s->it->kind == TOKEN_PTR_OP ? POSTFIX_PTR_ACCESS
                                                          : POSTFIX_ACCESS;
    accept_it(s);
    if (s->it->kind != TOKEN_IDENTIFIER) {
        return false;
    }
    const struct str spelling = token_take_spelling(s->it);
    struct source_loc loc = s->it->loc;
    accept_it(s);
    struct identifier* identifier = create_identifier(&spelling, loc);
    res->kind = kind;
    res->identifier = identifier;
    return true;
}

struct postfix_suffix parse_postfix_inc_dec_suffix(struct parser_state* s) {
    assert(s->it->kind == TOKEN_INC|| s->it->kind == TOKEN_DEC);
    const enum token_kind op = s->it->kind;
    accept_it(s);
    return (struct postfix_suffix){
        .kind = op == TOKEN_INC ? POSTFIX_INC : POSTFIX_DEC,
    };
}

static bool parse_postfix_suffixes(struct parser_state* s,
                                   struct postfix_expr* res) {
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

struct postfix_expr* parse_postfix_expr(struct parser_state* s) {
    struct postfix_expr* res = mycc_alloc(sizeof *res);
    res->suffixes = NULL;
    res->len = 0;

    if (s->it->kind == TOKEN_LBRACKET && next_is_type_name(s)) {
        res->info = create_ast_node_info(s->it->loc);
        accept_it(s);

        res->is_primary = false;

        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            mycc_free(res);
            return NULL;
        }

        if (!(accept(s, TOKEN_RBRACKET) && accept(s, TOKEN_LBRACE))) {
            free_type_name(res->type_name);
            mycc_free(res);
            return NULL;
        }

        if (!parse_init_list(s, &res->init_list)) {
            free_type_name(res->type_name);
            mycc_free(res);
            return NULL;
        }

        if (s->it->kind == TOKEN_COMMA) {
            accept_it(s);
        }

        if (!accept(s, TOKEN_RBRACE)) {
            free_postfix_expr(res);
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
        free_postfix_expr(res);
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
struct postfix_expr* parse_postfix_expr_type_name(
    struct parser_state* s,
    struct type_name* type_name,
    struct source_loc start_bracket_loc) {
    assert(type_name);
    assert(s->it->kind == TOKEN_LBRACE);

    struct postfix_expr* res = mycc_alloc(sizeof *res);
    res->len = 0;
    res->suffixes = NULL;
    res->is_primary = false;
    res->info = create_ast_node_info(start_bracket_loc);
    res->type_name = type_name;

    res->init_list.len = 0;
    res->init_list.inits = NULL;

    accept_it(s);

    if (!parse_init_list(s, &res->init_list)) {
        goto fail;
    }

    if (s->it->kind == TOKEN_COMMA) {
        accept_it(s);
    }

    if (!accept(s, TOKEN_RBRACE)) {
        return NULL;
    }

    if (!parse_postfix_suffixes(s, res)) {
        goto fail;
    }
    return res;
fail:
    free_postfix_expr(res);
    return NULL;
}

static void free_children(struct postfix_expr* p) {
    if (p->is_primary) {
        free_primary_expr(p->primary);
    } else {
        free_type_name(p->type_name);
        free_init_list_children(&p->init_list);
    }
    for (size_t i = 0; i < p->len; ++i) {
        struct postfix_suffix* s = &p->suffixes[i];
        switch (s->kind) {
            case POSTFIX_INDEX:
                free_expr(s->index_expr);
                break;
            case POSTFIX_BRACKET:
                free_arg_expr_list(&s->bracket_list);
                break;
            case POSTFIX_ACCESS:
            case POSTFIX_PTR_ACCESS:
                free_identifier(s->identifier);
                break;
            case POSTFIX_INC:
            case POSTFIX_DEC:
                break;
        }
    }
    mycc_free(p->suffixes);
}

void free_postfix_expr(struct postfix_expr* p) {
    free_children(p);
    mycc_free(p);
}

