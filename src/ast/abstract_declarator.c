#include "ast/abstract_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

AbstractDeclarator* create_abstract_declarator(Pointer* ptr, DirectAbstractDeclarator* direct_abs_decl) {
    assert(ptr || direct_abs_decl); 
    AbstractDeclarator* res = xmalloc(sizeof(AbstractDeclarator));
    res->ptr = ptr;
    res->direct_abs_decl = direct_abs_decl;
    return res;
}

static void free_children(AbstractDeclarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    if (d->direct_abs_decl) {
        free_direct_abstract_declarator(d->direct_abs_decl);
    }
}

void free_abstract_declarator(AbstractDeclarator* d) {
    free_children(d);
    free(d);
}

