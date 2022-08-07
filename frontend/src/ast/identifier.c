#include "frontend/ast/identifier.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

void init_identifier(struct identifier* res, char* spelling, struct source_loc loc) {
    assert(res);
    res->info = create_ast_node_info(loc);
    res->spelling = spelling;
}

struct identifier* create_identifier(char* spelling, struct source_loc loc) {
    struct identifier* res = xmalloc(sizeof(struct identifier));
    init_identifier(res, spelling, loc);
    return res;
}

void free_identifier_children(struct identifier* i) {
    free(i->spelling);
}

void free_identifier(struct identifier* i) {
    free_identifier_children(i);
    free(i);
}

