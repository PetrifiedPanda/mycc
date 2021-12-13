#include "ast/abstract_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct abstract_declarator* create_abstract_declarator(struct pointer* ptr, struct direct_abstract_declarator* direct_abs_decl) {
    assert(ptr || direct_abs_decl); 
    struct abstract_declarator* res = xmalloc(sizeof(struct abstract_declarator));
    res->ptr = ptr;
    res->direct_abs_decl = direct_abs_decl;
    return res;
}

static void free_children(struct abstract_declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    if (d->direct_abs_decl) {
        free_direct_abstract_declarator(d->direct_abs_decl);
    }
}

void free_abstract_declarator(struct abstract_declarator* d) {
    free_children(d);
    free(d);
}

