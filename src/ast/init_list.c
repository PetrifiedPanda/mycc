#include "ast/init_list.h"

#include <assert.h>

InitList create_init_list(Initializer* inits, size_t len) {
    assert(len > 0);
    assert(inits);
    
    InitList res;
    
    res.len = len;
    res.inits = inits; 
    return res;
}

void free_init_list_children(InitList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_initializer_children(&l->inits[i]);
    }
    free(l->inits);
}

