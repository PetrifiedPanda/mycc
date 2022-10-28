#include "frontend/ast/initializer/designation.h"

#include <stdlib.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

struct designation* parse_designation(struct parser_state* s) {
    struct designator_list designators = parse_designator_list(s);
    if (designators.len == 0) {
        return NULL;
    }

    if (!accept(s, ASSIGN)) {
        free_designator_list(&designators);
        return NULL;
    }

    struct designation* res = xmalloc(sizeof *res);
    res->designators = designators;
    return res;
}

static void free_children(struct designation* d) {
    free_designator_list(&d->designators);
}

void free_designation(struct designation* d) {
    free_children(d);
    free(d);
}
