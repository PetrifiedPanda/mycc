#include "ast/enumerator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

Enumerator* create_enumerator(Identifier* identifier, ConstExpr* enum_val) {
    assert(identifier);
    Enumerator* res = xmalloc(sizeof(Enumerator));
    res->identifier = identifier;
    res->enum_val = enum_val;
    
    return res;
}

void free_enumerator_children(Enumerator* e) {
    free_identifier(e->identifier);
    if (e->enum_val) {
        free_const_expr(e->enum_val);
    }
}

