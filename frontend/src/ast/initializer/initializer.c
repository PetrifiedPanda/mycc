#include "frontend/ast/initializer/initializer.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static struct initializer* create_initializer_assign(
    struct source_loc loc,    
    struct assign_expr* assign) {
    assert(assign);
    struct initializer* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->is_assign = true;
    res->assign = assign;

    return res;
}

static struct initializer* create_initializer_init_list(
    struct source_loc loc,
    struct init_list init_list) {
    struct initializer* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->is_assign = false;
    res->init_list = init_list;

    return res;
}

struct initializer* parse_initializer(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (s->it->type == LBRACE) {
        accept_it(s);
        struct init_list init_list;
        if (!parse_init_list(s, &init_list)) {
            return NULL;
        }

        if (s->it->type == COMMA) {
            accept_it(s);
        }

        if (!accept(s, RBRACE)) {
            free_init_list_children(&init_list);
        }
        return create_initializer_init_list(loc, init_list);
    } else {
        struct assign_expr* assign = parse_assign_expr(s);
        if (!assign) {
            return NULL;
        }
        return create_initializer_assign(loc, assign);
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
