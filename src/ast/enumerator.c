#include "ast/enumerator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct enumerator* create_enumerator(struct identifier* identifier, struct const_expr* enum_val) {
    assert(identifier);
    struct enumerator* res = xmalloc(sizeof(struct enumerator));
    res->identifier = identifier;
    res->enum_val = enum_val;
    
    return res;
}

void free_enumerator_children(struct enumerator* e) {
    free_identifier(e->identifier);
    if (e->enum_val) {
        free_const_expr(e->enum_val);
    }
}

