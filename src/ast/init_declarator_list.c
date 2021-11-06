#include "ast/init_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

InitDeclaratorList create_init_declarator_list(InitDeclarator* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }

    InitDeclaratorList res;
    
    res.len = len;
    res.decls = decls;
    return res;
}

void free_init_declarator_list(InitDeclaratorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_init_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

