#include "ast/declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct declarator* create_declarator(struct pointer* ptr, struct direct_declarator* direct_decl) {
    assert(direct_decl);
    struct declarator* res = xmalloc(sizeof(struct declarator));
    res->ptr = ptr;
    res->direct_decl = direct_decl;
    
    return res;
}

static void free_children(struct declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declarator(struct declarator* d) {
    free_children(d);
    free(d);
}

