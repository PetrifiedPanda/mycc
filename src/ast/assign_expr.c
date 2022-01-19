#include "ast/assign_expr.h"

#include <assert.h>
#include <stdlib.h>

#include "util.h"

#include "parser/parser_util.h"

bool parse_assign_expr_inplace(struct parser_state* s, struct assign_expr* res) {
    assert(res);

    struct unary_expr* last_unary = parse_unary_expr(s);
    if (!last_unary) {
        return false;
    }

    size_t alloc_len = res->len = 0;
    res->assign_chain = NULL;
    res->value = NULL;

    while (is_assign_op(s->it->type)) {
        enum token_type op = s->it->type;
        accept_it(s);

        struct unary_expr* new_last = parse_unary_expr(s);
        if (!last_unary) {
            free_unary_expr(last_unary);
            goto fail;
        }

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->assign_chain, &alloc_len, sizeof(struct unary_and_op));
        }

        res->assign_chain[res->len] = (struct unary_and_op){
                .assign_op = op,
                .unary = last_unary
        };
        last_unary = new_last;

        ++res->len;
    }

    res->assign_chain = xrealloc(res->assign_chain, sizeof(struct unary_and_op) * res->len);

    res->value = parse_cond_expr_unary(s, last_unary);
    if (!res->value) {
        goto fail;
    }

    return true;
fail:
    for (size_t i = 0; i < res->len; ++i) {
        free_unary_expr(res->assign_chain[i].unary);
    }

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
        free_unary_expr_children(e->assign_chain[i].unary);
    }
    free(e->assign_chain);
    
    free_cond_expr(e->value);
}

void free_assign_expr(struct assign_expr* e) {
    free_assign_expr_children(e);
    free(e);
}

