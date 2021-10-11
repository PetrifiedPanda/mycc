#include "ast/spec_qual_list.h"

#include <stdlib.h>

static void free_children(SpecQualList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        TypeSpecOrQual* item = &l->specs_or_quals[i];
        if (item->is_type_spec) {
            free_type_spec(item->type_spec);
        }
    }
}

void free_spec_qual_list(SpecQualList* l) {
    free_children(l);
    free(l);
}
