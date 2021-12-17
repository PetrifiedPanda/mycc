#include "ast/generic_sel.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct generic_sel* create_generic_sel(struct assign_expr* assign, struct generic_assoc_list assocs) {
    assert(assign);
    assert(assocs.len != 0);
    struct generic_sel* res = xmalloc(sizeof(struct generic_sel));

    res->assign = assign;
    res->assocs = assocs;
    return res;
}

static void free_children(struct generic_sel* s) {
    free_assign_expr(s->assign);
    free_generic_assoc_list(&s->assocs);
}

void free_generic_sel(struct generic_sel* s) {
    free_children(s);
    free(s);
}
