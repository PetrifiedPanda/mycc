#include "ast/declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

Declarator* create_declarator(Pointer* ptr, DirectDeclarator* direct_decl) {
    assert(direct_decl);
    Declarator* res = xmalloc(sizeof(Declarator));
    res->ptr = ptr;
    res->direct_decl = direct_decl;
    
    return res;
}

static void free_children(Declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declarator(Declarator* d) {
    free_children(d);
    free(d);
}

