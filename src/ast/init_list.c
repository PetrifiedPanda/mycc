#include "ast/init_list.h"

#include <stdlib.h>
#include <assert.h>

InitList* create_init_list(Initializer* inits, size_t len) {
    assert(len > 0);
    assert(inits);
    InitList* res = malloc(sizeof(InitList));
    if (res) {
        res->len = len;
        res->inits = inits;
    }
    return res;
}

static void free_children(InitList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_initializer_children(&l->inits[i]);
    }
    free(l->inits);
}

void free_init_list(InitList* l) {
    free_children(l);
    free(l);
}
