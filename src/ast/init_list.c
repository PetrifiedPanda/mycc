#include "ast/init_list.h"

#include <assert.h>

static struct init_list create_init_list(struct designation_init* inits, size_t len) {
    assert(len > 0);
    assert(inits);
    return (struct init_list){.len = len, .inits = inits};
}

struct init_list parse_init_list(struct parser_state* s) {
    (void)s;
    // TODO:
    return (struct init_list){.len = 0, .inits = NULL};
}

void free_init_list_children(struct init_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        struct designation_init* item = &l->inits[i];
        if (item->designation) {
            free_designation(item->designation);
        }
        free_initializer(item->init);
    }
    free(l->inits);
}

