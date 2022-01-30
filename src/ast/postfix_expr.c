#include "ast/postfix_expr.h"

#include <assert.h>

#include "util.h"
#include "error.h"

#include "parser/parser_util.h"

static bool is_posfix_op(enum token_type t) {
    switch (t) {
        case LINDEX:
        case LBRACKET:
        case DOT:
        case PTR_OP:
        case INC_OP:
        case DEC_OP:
            return true;

        default:
            return false;
    }
}

static bool parse_postfix_suffixes(struct parser_state* s, struct postfix_expr* res) {
    size_t alloc_len = 0;
    while (is_posfix_op(s->it->type)) {
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->suffixes, &alloc_len, sizeof(struct postfix_suffix));
        }

        switch (s->it->type) {
            case LINDEX: {
                accept_it(s);
                struct expr* expr = parse_expr(s);
                if (!expr) {
                    return false;
                }
                if (!accept(s, RINDEX)) {
                    free_expr(expr);
                    return false;
                }
                res->suffixes[res->len] = (struct postfix_suffix){
                        .type = POSTFIX_INDEX,
                        .index_expr = expr
                };
                break;
            }

            case LBRACKET: {
                accept_it(s);
                struct arg_expr_list arg_expr_list = {.assign_exprs = NULL, .len = 0};
                if (s->it->type != RBRACKET) {
                    arg_expr_list = parse_arg_expr_list(s);
                    if (arg_expr_list.len == 0) {
                        return false;
                    }
                }
                if (!accept(s, RBRACKET)) {
                    free_arg_expr_list(&arg_expr_list);
                    return false;
                }
                res->suffixes[res->len] = (struct postfix_suffix){
                        .type = POSTFIX_BRACKET,
                        .bracket_list = arg_expr_list
                };
                break;
            }

            case DOT:
            case PTR_OP: {
                enum postfix_suffix_type type = s->it->type == PTR_OP
                                                ? POSTFIX_PTR_ACCESS : POSTFIX_ACCESS;
                accept_it(s);
                if (s->it->type != IDENTIFIER) {
                    return false;
                }
                char* spelling = take_spelling(s->it);
                accept_it(s);
                struct identifier* identifier = create_identifier(spelling);
                res->suffixes[res->len] = (struct postfix_suffix){
                        .type = type,
                        .identifier = identifier
                };
                break;
            }

            case INC_OP:
            case DEC_OP: {
                enum token_type inc_dec = s->it->type;
                accept_it(s);
                res->suffixes[res->len] = (struct postfix_suffix){
                        .type = POSTFIX_INC_DEC,
                        .inc_dec = inc_dec
                };
                break;
            }

            default:
                assert(false); // Unreachable
        }

        ++res->len;
    }

    if (alloc_len != res->len) {
        res->suffixes = xrealloc(res->suffixes, res->len * sizeof(struct postfix_suffix));
    }

    return true;
}

struct postfix_expr* parse_postfix_expr(struct parser_state* s) {
    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->suffixes = NULL;
    res->len = 0;

    if (s->it->type == LBRACKET && next_is_type_name(s)) {
        accept_it(s);

        res->is_primary = false;

        res->type_name = parse_type_name(s);
        if (!res->type_name) {
            free(res);
            return NULL;
        }

        if (!(accept(s, RBRACKET) && accept(s, LBRACE))) {
            free_type_name(res->type_name);
            free(res);
            return NULL;
        }

        res->init_list = parse_init_list(s);
        if (res->init_list.len == 0) {
            free_type_name(res->type_name);
            free(res);
            return NULL;
        }

        if (!accept(s, RBRACE)) {
            free_postfix_expr(res);
            return NULL;
        }
    } else {
        res->is_primary = true;
        res->primary = parse_primary_expr(s);
        if (!res->primary) {
            free(res);
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
struct postfix_expr* parse_postfix_expr_type_name(struct parser_state* s, struct type_name* type_name) {
    assert(type_name);
    assert(s->it->type == LBRACE);

    struct postfix_expr* res = xmalloc(sizeof(struct postfix_expr));
    res->len = 0;
    res->suffixes = NULL;
    res->is_primary = false;
    res->type_name = type_name;

    res->init_list.len = 0;
    res->init_list.inits = NULL;

    accept_it(s);

    res->init_list = parse_init_list(s);
    if (get_last_error() != ERR_NONE) {
        goto fail;
    }

    if (!accept(s, RBRACE)) {
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
        switch (s->type) {
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
            case POSTFIX_INC_DEC:
                break;
        }
    }
    free(p->suffixes);
}

void free_postfix_expr(struct postfix_expr* p) {
    free_children(p);
    free(p);
}

