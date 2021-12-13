#include "ast/type_qual_list.h"

#include <stdlib.h>
#include <assert.h>

struct type_qual_list create_type_qual_list(struct type_qual* type_quals, size_t len) {
    if (len > 0) {
        assert(type_quals);
    } else {
        assert(type_quals == NULL);
    }

    struct type_qual_list res;

    res.len = len;
    res.type_quals = type_quals;
    return res;
}

void free_type_qual_list(struct type_qual_list* l) {
    free(l->type_quals);
}

