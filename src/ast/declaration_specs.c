#include "ast/declaration_specs.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct declaration_specs* create_declaration_specs(struct declaration_specs_cont* contents, size_t len) {
    assert(len > 0);
    assert(contents);
    for (size_t i = 0; i < len; ++i) {
        struct declaration_specs_cont* item = &contents[i];
        if (item->type == DECLSPEC_STORAGE_CLASS_SPEC) {
            assert(is_storage_class_spec(item->storage_class_spec));
        }
    }
    struct declaration_specs* res = xmalloc(sizeof(struct declaration_specs));
    res->len = len;
    res->contents = contents;
    
    return res;
}

static void free_children(struct declaration_specs* s) {
    for (size_t i = 0; i < s->len; ++i) {
        struct declaration_specs_cont* item = &s->contents[i];
        if (item->type == DECLSPEC_TYPE_SPEC) {
            free_type_spec(item->type_spec);
        }
    }
    free(s->contents);
}

void free_declaration_specs(struct declaration_specs* s) {
    free_children(s);
    free(s);
}

