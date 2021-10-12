#include "ast/enum_list.h"

#include <stdlib.h>
#include <assert.h>

EnumList* create_enum_list(Enumerator* enums, size_t len) {
    assert(len > 0);
    assert(enums);
    EnumList* res = malloc(sizeof(EnumList));
    if (res) {
        res->len = len;
        res->enums = enums;
    }
    return res;
}

static void free_children(EnumList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_enumerator_children(&l->enums[i]);
    }
    free(l->enums);
}

void free_enum_list(EnumList* l) {
    free_children(l);
    free(l);
}
