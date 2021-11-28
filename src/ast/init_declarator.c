#include "ast/init_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

InitDeclarator* create_init_declarator(Declarator* decl, Initializer* init) {
    assert(decl);
    InitDeclarator* res = xmalloc(sizeof(InitDeclarator));
    res->decl = decl;
    res->init = init;
    
    return res;
}

void free_init_declarator_children(InitDeclarator* d) {
    free_declarator(d->decl);
    free_initializer(d->init);
}

