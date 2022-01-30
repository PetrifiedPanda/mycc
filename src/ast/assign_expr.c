#include "ast/assign_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_assign_expr_inplace(struct parser_state* s, struct assign_expr* res) {
    assert(res);

    struct unary_expr* last_unary;
    if (s->it->type == LBRACKET && next_is_type_name(s)) {
        accept_it(s);
        struct type_name* type_name = parse_type_name(s);
        if (!type_name) {
            return false;
        }

        if (!accept(s, RBRACKET)) {
            free_type_name(type_name);
            return false;
        }

        if (s->it->type == LBRACE) {
            last_unary = parse_unary_expr_type_name(s, NULL, 0, type_name);
        } else {
            res->len = 0;
            res->assign_chain = NULL;
            struct cast_expr* cast_expr = parse_cast_expr_type_name(s, type_name);
            if (!cast_expr) {
                return false;
            }

            res->value = parse_cond_expr_cast(s, cast_expr);
            if (!res->value) {
                return false;
            }

            return true;
        }
    } else {
        last_unary = parse_unary_expr(s);
    }
    if (!last_unary) {
        return false;
    }

    size_t alloc_len = res->len = 0;
    res->assign_chain = NULL;
    res->value = NULL;

    while (is_assign_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        struct unary_expr* new_last;
        if (s->it->type == LBRACKET && next_is_type_name(s)) {
            accept_it(s);

            struct type_name* type_name = parse_type_name(s);
            if (!type_name) {
                free_unary_expr(last_unary);
                goto fail;
            }

            if (!accept(s, RBRACKET)) {
                free_unary_expr(last_unary);
                free_type_name(type_name);
                goto fail;
            }

            if (s->it->type == LBRACE) {
                new_last = parse_unary_expr_type_name(s, NULL, 0, type_name);
            } else {
                struct cast_expr* cast_expr = parse_cast_expr_type_name(s, type_name);
                if (!cast_expr) {
                    free_unary_expr(last_unary);
                    goto fail;
                }

                res->value = parse_cond_expr_cast(s, cast_expr);
                if (!res->value) {
                    free_unary_expr(last_unary);
                    free_cast_expr(cast_expr);
                    goto fail;
                }
                ++res->len;
                res->assign_chain = xrealloc(res->assign_chain, sizeof(struct cond_expr) * res->len);
                res->assign_chain[res->len - 1] = (struct unary_and_op) {
                    .assign_op = op,
                    .unary = last_unary
                };
                return true;
            }
        } else {
            new_last = parse_unary_expr(s);
        }

        if (!new_last) {
            free_unary_expr(last_unary);
            goto fail;
        }

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->assign_chain, &alloc_len, sizeof(struct unary_and_op));
        }

        res->assign_chain[res->len] = (struct unary_and_op) {
                .assign_op = op,
                .unary = last_unary
        };
        last_unary = new_last;

        ++res->len;
    }

    res->assign_chain = xrealloc(res->assign_chain, sizeof(struct unary_and_op) * res->len);

    res->value = parse_cond_expr_cast(s, create_cast_expr_unary(last_unary));
    if (!res->value) {
        goto fail;
    }

    return true;
fail:
    for (size_t i = 0; i < res->len; ++i) {
        free_unary_expr(res->assign_chain[i].unary);
    }
    free(res->assign_chain);

    return false;
}

struct assign_expr* parse_assign_expr(struct parser_state* s) {
    struct assign_expr* res = xmalloc(sizeof(struct assign_expr));
    if (!parse_assign_expr_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_assign_expr_children(struct assign_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_unary_expr(e->assign_chain[i].unary);
    }
    free(e->assign_chain);
    
    free_cond_expr(e->value);
}

void free_assign_expr(struct assign_expr* e) {
    free_assign_expr_children(e);
    free(e);
}

