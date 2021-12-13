#include "ast/abs_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct abs_declarator* create_abs_declarator(struct pointer* ptr, struct direct_abs_declarator* direct_abs_decl) {
    assert(ptr || direct_abs_decl); 
    struct abs_declarator* res = xmalloc(sizeof(struct abs_declarator));
    res->ptr = ptr;
    res->direct_abs_decl = direct_abs_decl;
    return res;
}

static void free_children(struct abs_declarator* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    if (d->direct_abs_decl) {
        free_direct_abs_declarator(d->direct_abs_decl);
    }
}

void free_abs_declarator(struct abs_declarator* d) {
    free_children(d);
    free(d);
}

