#include "ast/translation_unit.h"

#include <stdlib.h>
#include <assert.h>

static void free_children(struct translation_unit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        free_external_declaration_children(&u->external_decls[i]);
    }
    free(u->external_decls);
}

void free_translation_unit(struct translation_unit* u) {
    free_children(u);
}

