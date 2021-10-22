#include "ast/param_list.h"

#include <stdlib.h>
#include <assert.h>

ParamList* create_param_list(ParamDeclaration* decls, size_t len) {
    assert(len > 0);
    assert(decls);
    ParamList* res = malloc(sizeof(ParamList));
    if (res) {
        res->len = len;
        res->decls = decls;
    }
    return res;
}

static void free_children(ParamList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_param_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

void free_param_list(ParamList* l) {
    free_children(l);
    free(l);
}
