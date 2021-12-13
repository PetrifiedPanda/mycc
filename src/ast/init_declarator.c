#include "ast/init_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct init_declarator* create_init_declarator(struct declarator* decl, struct initializer* init) {
    assert(decl);
    struct init_declarator* res = xmalloc(sizeof(struct init_declarator));
    res->decl = decl;
    res->init = init;
    
    return res;
}

void free_init_declarator_children(struct init_declarator* d) {
    free_declarator(d->decl);
    free_initializer(d->init);
}

