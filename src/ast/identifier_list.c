#include "ast/identifier_list.h"

#include <stdlib.h>
#include <assert.h>

IdentifierList* create_identifier_list(char** identifiers, size_t len) {
    assert(len > 0);
    assert(identifiers);
    IdentifierList* res = malloc(sizeof(IdentifierList));
    if (res) {
        res->len = len;
        res->identifiers = identifiers;
    }
    return res;
}

static void free_children(IdentifierList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free(l->identifiers[i]);
    }
    free(l->identifiers);
}

void free_identifier_list(IdentifierList* l) {
    free_children(l);
    free(l);
}