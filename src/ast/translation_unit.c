#include "ast/translation_unit.h"

#include <stdlib.h>
#include <assert.h>

TranslationUnit* create_translation_unit(ExternalDeclaration* external_decls, size_t len) {
    assert(len > 0);
    assert(external_decls);
    TranslationUnit* res = malloc(sizeof(TranslationUnit));
    if (res) {
        res->len = len;
        res->external_decls = external_decls;
    }
    return res;
}

static void free_children(TranslationUnit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        free_external_declaration_children(&u->external_decls[i]);
    }
    free(u->external_decls);
}

void free_translation_unit(TranslationUnit* u) {
    free_children(u);
    free(u);
}
