#include "ast/enumerator.h"

#include <stdlib.h>
#include <assert.h>

Enumerator* create_enumerator(char* identifier, ConstExpr* enum_val) {
    assert(identifier);
    Enumerator* res = malloc(sizeof(Enumerator));
    if (res) {
        res->identifier = identifier;
        res->enum_val = enum_val;
    }
    return res;
}

void free_enumerator_children(Enumerator* e) {
    free(e->identifier);
    if (e->enum_val) {
        free_const_expr(e->enum_val);
    }
}