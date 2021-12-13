#include "ast/enum_list.h"

#include <stdlib.h>
#include <assert.h>

struct enum_list create_enum_list(struct enumerator* enums, size_t len) {
    if (len > 0) {
        assert(enums);
    } else {
        assert(enums == NULL);
    }

    struct enum_list res;

    res.len = len;
    res.enums = enums;
    return res;
}

void free_enum_list(struct enum_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_enumerator_children(&l->enums[i]);
    }
    free(l->enums);
}

