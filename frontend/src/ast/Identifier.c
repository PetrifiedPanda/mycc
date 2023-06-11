#include "frontend/ast/Identifier.h"

#include <assert.h>

#include "util/mem.h"

void Identifier_init(Identifier* res, const StrBuf* spelling, SourceLoc loc) {
    assert(res);
    res->info = AstNodeInfo_create(loc);
    res->spelling = *spelling;
}

Identifier* Identifier_create(const StrBuf* spelling, SourceLoc loc) {
    Identifier* res = mycc_alloc(sizeof *res);
    Identifier_init(res, spelling, loc);
    return res;
}

void Identifier_free_children(Identifier* i) {
    StrBuf_free(&i->spelling);
}

void Identifier_free(Identifier* i) {
    Identifier_free_children(i);
    mycc_free(i);
}

