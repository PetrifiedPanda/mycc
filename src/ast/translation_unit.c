#include "ast/translation_unit.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct translation_unit* create_translation_unit(struct external_declaration* external_decls, size_t len) {
    assert(len > 0);
    assert(external_decls);
    struct translation_unit* res = xmalloc(sizeof(struct translation_unit));
    res->len = len;
    res->external_decls = external_decls;
    
    return res;
}

static void free_children(struct translation_unit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        free_external_declaration_children(&u->external_decls[i]);
    }
    free(u->external_decls);
}

void free_translation_unit(struct translation_unit* u) {
    free_children(u);
    free(u);
}

