#include "frontend/ast/Identifier.h"

#include <assert.h>

#include "util/mem.h"

void Identifier_init(Identifier* res, uint32_t token_idx) {
    assert(res);
    res->info = AstNodeInfo_create(token_idx);
}

Identifier* Identifier_create(uint32_t token_idx) {
    Identifier* res = mycc_alloc(sizeof *res);
    Identifier_init(res, token_idx);
    return res;
}

void Identifier_free(Identifier* i) {
    mycc_free(i);
}

