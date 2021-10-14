#include "ast/type_qual_list.h"

#include <stdlib.h>
#include <assert.h>

TypeQualList create_type_qual_list(TypeQual* type_quals, size_t len) {
    if (len > 0) {
        assert(type_quals);
    } else {
        assert(type_quals == NULL);
    }

    TypeQualList res;

    res.len = len;
    res.type_quals = type_quals;
    return res;
}

void free_type_qual_list(TypeQualList* l) {
    free(l->type_quals);
}
