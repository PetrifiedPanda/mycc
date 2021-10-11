#include "ast/struct_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

StructDeclaratorList* create_struct_declarator_list(StructDeclarator* decls, size_t len) {
    assert(len > 0);
    assert(decls);
    StructDeclaratorList* res = malloc(sizeof(StructDeclaratorList));
    if (res) {
        res->len = len;
        res->decls = decls;
    }
    return res;
}

static void free_children(StructDeclaratorList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

void free_struct_declarator_list(StructDeclaratorList* l) {
    free_children(l);
    free(l);
}