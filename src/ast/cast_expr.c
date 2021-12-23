#include "ast/cast_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static struct cast_expr* create_cast_expr(struct type_name* type_names, size_t len, struct unary_expr* rhs) {
    assert(rhs);
    if (len > 0) {
        assert(type_names);
    } else {
        assert(type_names == NULL);
    }
    struct cast_expr* res = xmalloc(sizeof(struct cast_expr));
    res->type_names = type_names;
    res->len = len;
    res->rhs = rhs;
    return res;
}

struct cast_expr* parse_cast_expr(struct parser_state* s) {
    struct type_name* type_names = NULL;
    size_t len = 0;

    size_t alloc_size = 0;
    while (s->it->type == LBRACKET && next_is_type_name(s)) {
        accept_it(s);

        if (len == alloc_size) {
            grow_alloc((void**)&type_names, &alloc_size, sizeof(struct type_name));
        }

        if (!parse_type_name_inplace(s, &type_names[len])) {
            goto fail;
        }

        if (!accept(s, LBRACKET)) {
            goto fail;
        }
        ++len;
    }

    struct unary_expr* rhs;
    if (s->it->type == LBRACE) {
        struct type_name* type_name = xmalloc(sizeof(struct type_name));

        --len;
        *type_name = type_names[len];

        if (type_names) {
            type_names = xrealloc(type_names, len * sizeof(struct type_name));
        }

        rhs = parse_unary_expr_type_name(s, NULL, 0, type_name);
    } else {
        if (type_names) {
            type_names = xrealloc(type_names, len * sizeof(struct type_name));
        }
        rhs = parse_unary_expr(s);
    }
    if (!rhs) {
        goto fail;
    }

    return create_cast_expr(type_names, len, rhs);

    fail:
    for (size_t i = 0; i < len; ++i) {
        free_type_name_children(&type_names[i]);
    }
    free(type_names);
    return NULL;
}

static void free_children(struct cast_expr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_type_name_children(&e->type_names[i]);
    }
    free(e->type_names);
    free_unary_expr(e->rhs);
}

void free_cast_expr(struct cast_expr* e) {
    free_children(e);
    free(e);
}

