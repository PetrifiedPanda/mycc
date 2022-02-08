#include "ast/initializer.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static struct initializer* create_initializer_assign(
    struct assign_expr* assign) {
    assert(assign);
    struct initializer* res = xmalloc(sizeof(struct initializer));
    res->is_assign = true;
    res->assign = assign;

    return res;
}

static struct initializer* create_initializer_init_list(
    struct init_list init_list) {
    struct initializer* res = xmalloc(sizeof(struct initializer));
    res->is_assign = false;
    res->init_list = init_list;

    return res;
}

struct initializer* parse_initializer(struct parser_state* s) {
    if (s->it->type == LBRACE) {
        accept_it(s);
        struct init_list init_list = parse_init_list(s);
        if (init_list.len == 0) {
            return NULL;
        }

        if (s->it->type == COMMA) {
            accept_it(s);
        }

        if (!accept(s, RBRACE)) {
            free_init_list_children(&init_list);
        }
        return create_initializer_init_list(init_list);
    } else {
        struct assign_expr* assign = parse_assign_expr(s);
        if (!assign) {
            return NULL;
        }
        return create_initializer_assign(assign);
    }
}

void free_initializer_children(struct initializer* i) {
    if (i->is_assign) {
        free_assign_expr(i->assign);
    } else {
        free_init_list_children(&i->init_list);
    }
}

void free_initializer(struct initializer* i) {
    free_initializer_children(i);
    free(i);
}
