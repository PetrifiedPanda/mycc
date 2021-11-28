#include "ast/identifier_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

IdentifierList* create_identifier_list(char** identifiers, size_t len) {
    assert(len > 0);
    assert(identifiers);
    IdentifierList* res = xmalloc(sizeof(IdentifierList));
    res->len = len;
    res->identifiers = identifiers;

    return res;
}

void free_identifier_list(IdentifierList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free(l->identifiers[i]);
    }
    free(l->identifiers);
}

