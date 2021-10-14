#include "ast/struct_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

StructDeclaratorList create_struct_declarator_list(StructDeclarator* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }

    StructDeclaratorList res;

    res.len = len;
    res.decls = decls;
    return res;
}

void free_struct_declarator_list(StructDeclaratorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}
