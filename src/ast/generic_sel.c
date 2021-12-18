#include "ast/generic_sel.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

#include "parser/parser_util.h"

static struct generic_sel* create_generic_sel(struct assign_expr* assign, struct generic_assoc_list assocs) {
    assert(assign);
    assert(assocs.len != 0);
    struct generic_sel* res = xmalloc(sizeof(struct generic_sel));

    res->assign = assign;
    res->assocs = assocs;
    return res;
}

struct generic_sel* parse_generic_sel(struct parser_state* s) {
    assert(s->it->type == GENERIC);
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

    return create_generic_sel(assign, assocs);

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
