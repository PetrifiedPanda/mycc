#include "ast/spec_qual_list.h"

#include <stdlib.h>
#include <assert.h>

SpecQualList create_spec_qual_list(TypeSpecOrQual* specs_or_quals, size_t len) {
    if (len > 0) {
        assert(specs_or_quals);
    } else {
        assert(specs_or_quals == NULL);
    }

    SpecQualList res;
    res.len = len;
    res.specs_or_quals = specs_or_quals;
    return res;
}

void free_spec_qual_list(SpecQualList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        TypeSpecOrQual* item = &l->specs_or_quals[i];
        if (item->is_type_spec) {
            free_type_spec(item->type_spec);
        }
    }
}

