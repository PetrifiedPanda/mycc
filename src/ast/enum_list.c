#include "ast/enum_list.h"

#include <stdlib.h>
#include <assert.h>

EnumList create_enum_list(Enumerator* enums, size_t len) {
    if (len > 0) {
        assert(enums);
    } else {
        assert(enums == NULL);
    }

    EnumList res;

    res.len = len;
    res.enums = enums;
    return res;
}

void free_enum_list(EnumList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_enumerator_children(&l->enums[i]);
    }
    free(l->enums);
}
