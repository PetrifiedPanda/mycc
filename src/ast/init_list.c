#include "ast/init_list.h"

#include <assert.h>

struct init_list create_init_list(struct designation_init* inits, size_t len) {
    assert(len > 0);
    assert(inits);
    return (struct init_list){.len = len, .inits = inits};
}

void free_init_list_children(struct init_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_initializer_children(&l->inits[i]);
    }
    free(l->inits);
}

