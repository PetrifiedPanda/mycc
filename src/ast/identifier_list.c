#include "ast/identifier_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct identifier_list create_identifier_list(struct identifier* identifiers, size_t len) {
    assert(len > 0);
    assert(identifiers);
    return (struct identifier_list){.len = len, .identifiers = identifiers};
}

void free_identifier_list(struct identifier_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_identifier_children(&l->identifiers[i]);
    }
    free(l->identifiers);
}

