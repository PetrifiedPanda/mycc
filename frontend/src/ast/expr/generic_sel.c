#include "frontend/ast/expr/generic_sel.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static struct generic_sel* create_generic_sel(
    struct assign_expr* assign,
    struct generic_assoc_list assocs,
    struct source_loc loc) {
    assert(assign);
    assert(assocs.len != 0);
    struct generic_sel* res = xmalloc(sizeof *res);
    
    res->info = create_ast_node_info(loc);
    res->assign = assign;
    res->assocs = assocs;
    return res;
}

struct generic_sel* parse_generic_sel(struct parser_state* s) {
    assert(s->it->type == GENERIC);
    struct source_loc loc = s->it->loc;
    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct assign_expr* assign = parse_assign_expr(s);
    if (!assign) {
        return NULL;
    }

    if (!accept(s, COMMA)) {
        goto fail;
    }

    struct generic_assoc_list assocs = parse_generic_assoc_list(s);
    if (assocs.len == 0) {
        goto fail;
    }

    if (!accept(s, RBRACKET)) {
        free_generic_assoc_list(&assocs);
        goto fail;
    }

    return create_generic_sel(assign, assocs, loc);

fail:
    free_assign_expr(assign);
    return NULL;
}

static void free_children(struct generic_sel* s) {
    free_assign_expr(s->assign);
    free_generic_assoc_list(&s->assocs);
}

void free_generic_sel(struct generic_sel* s) {
    free_children(s);
    free(s);
}
