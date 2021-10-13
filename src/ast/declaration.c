#include "ast/declaration.h"

#include <stdlib.h>
#include <assert.h>

Declaration* create_declaration(Pointer* ptr, DirectDeclarator* direct_decl) {
    assert(direct_decl);
    Declaration* res = malloc(sizeof(Declaration));
    if (res) {
        res->ptr = ptr;
        res->direct_decl = direct_decl;
    }
    return res;
}

void free_declaration_children(Declaration* d) {
    if (d->ptr) {
        free_pointer(d->ptr);
    }
    free_direct_declarator(d->direct_decl);
}

void free_declaration(Declaration* d) {
    free_declaration_children(d);
    free(d);
}
