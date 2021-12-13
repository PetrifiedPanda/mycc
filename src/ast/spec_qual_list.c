#include "ast/spec_qual_list.h"

#include <stdlib.h>
#include <assert.h>

struct spec_qual_list create_spec_qual_list(struct type_spec_or_qual* specs_or_quals, size_t len) {
    if (len > 0) {
        assert(specs_or_quals);
    } else {
        assert(specs_or_quals == NULL);
    }

    struct spec_qual_list res;
    res.len = len;
    res.specs_or_quals = specs_or_quals;
    return res;
}

void free_spec_qual_list(struct spec_qual_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct type_spec_or_qual* item = &l->specs_or_quals[i];
        if (item->is_type_spec) {
            free_type_spec(item->type_spec);
        }
    }
}

