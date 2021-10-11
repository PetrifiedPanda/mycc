#include "ast/declaration_specs.h"

#include <stdlib.h>
#include <assert.h>

DeclarationSpecs* create_declaration_specs(DeclarationSpecsCont* contents, size_t len) {
    assert(len > 0);
    assert(contents);
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
        switch (item->type) {
            case DECLSPEC_TYPE_SPEC:
                free_type_spec(item->type_spec);
                break;
            case DECLSPEC_TYPE_QUAL:
                free_type_qual(item->type_qual);
                break;
        }
    }
    free(s->contents);
}

void free_declaration_specs(DeclarationSpecs* s) {
    free_children(s);
    free(s);
}
