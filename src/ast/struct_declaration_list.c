#include "ast/struct_declaration_list.h"

#include <stdlib.h>
#include <assert.h>

StructDeclarationList create_struct_declaration_list(StructDeclaration* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }

    StructDeclarationList res;

    res.len = len;
    res.decls = decls;
    return res;
}

void free_struct_declaration_list(StructDeclarationList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}
