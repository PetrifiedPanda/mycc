#include "frontend/ast/identifier.h"

#include <assert.h>

#include "util/mem.h"

void init_identifier(struct identifier* res, const struct str* spelling, struct source_loc loc) {
    assert(res);
    res->info = create_ast_node_info(loc);
    res->spelling = *spelling;
}

struct identifier* create_identifier(const struct str* spelling, struct source_loc loc) {
    struct identifier* res = mycc_alloc(sizeof *res);
    init_identifier(res, spelling, loc);
    return res;
}

void free_identifier_children(struct identifier* i) {
    free_str(&i->spelling);
}

void free_identifier(struct identifier* i) {
    free_identifier_children(i);
    mycc_free(i);
}

