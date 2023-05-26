#include "frontend/ast/Identifier.h"

#include <assert.h>

#include "util/mem.h"

void init_identifier(Identifier* res, const Str* spelling, SourceLoc loc) {
    assert(res);
    res->info = create_ast_node_info(loc);
    res->spelling = *spelling;
}

Identifier* create_identifier(const Str* spelling, SourceLoc loc) {
    Identifier* res = mycc_alloc(sizeof *res);
    init_identifier(res, spelling, loc);
    return res;
}

void free_identifier_children(Identifier* i) {
    Str_free(&i->spelling);
}

void free_identifier(Identifier* i) {
    free_identifier_children(i);
    mycc_free(i);
}

