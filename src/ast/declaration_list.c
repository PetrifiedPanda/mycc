#include "ast/declaration_list.h"

#include <stdlib.h>
#include <assert.h>

DeclarationList* create_declaration_list(Declaration* decls, size_t len) {
    assert(len > 0);
    assert(decls);
    DeclarationList* res = malloc(sizeof(DeclarationList));
    if (res) {
        res->len = len;
        res->decls = decls;
    }
    return res;
}

static void free_children(DeclarationList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

void free_declaration_list(DeclarationList* l) {
    free_children(l);
    free(l);
}
