#include "ast/type_qual_list.h"

#include <stdlib.h>
#include <assert.h>

TypeQualList* create_type_qual_list(TypeQual* type_quals, size_t len) {
    assert(len > 0);
    assert(type_quals);
    TypeQualList* res = malloc(sizeof(TypeQualList));
    if (res) {
        res->len = len;
        res->type_quals = type_quals;
    }
    return res;
}

static void free_children(TypeQualList* l) {
    free(l->type_quals); 
}

void free_type_qual_list(TypeQualList* l) {
    free_children(l);
    free(l);
}
