#include "ast/declaration_specs.h"

#include <stdlib.h>
#include <assert.h>

DeclarationSpecs* create_declaration_specs(DeclarationSpecsCont* contents, size_t len) {
    assert(len > 0);
    assert(contents);
    for (size_t i = 0; i < len; ++i) {
        DeclarationSpecsCont* item = &contents[i];
        if (item->type == DECLSPEC_STORAGE_CLASS_SPEC) {
            assert(is_storage_class_spec(item->storage_class_spec));
        }
    }
    DeclarationSpecs* res = malloc(sizeof(DeclarationSpecs));
    if (res) {
        res->len = len;
        res->contents = contents;
    }
    return res;
}

static void free_children(DeclarationSpecs* s) {
    for (size_t i = 0; i < s->len; ++i) {
        DeclarationSpecsCont* item = &s->contents[i];
        if (item->type == DECLSPEC_TYPE_SPEC) {
            free_type_spec(item->type_spec);
        }
    }
    free(s->contents);
}

void free_declaration_specs(DeclarationSpecs* s) {
    free_children(s);
    free(s);
}
