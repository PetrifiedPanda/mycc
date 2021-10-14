#include "ast/init_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

InitDeclaratorList* create_init_declarator_list(InitDeclarator* decls, size_t len) {
    assert(len > 0);
    assert(decls);
    InitDeclaratorList* res = malloc(sizeof(InitDeclaratorList));
    if (res) {
        res->len = len;
        res->decls = decls;
    }
    return res;
}

static void free_children(InitDeclaratorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_init_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

void free_init_declarator_list(InitDeclaratorList* l) {
    free_children(l);
    free(l);
}
