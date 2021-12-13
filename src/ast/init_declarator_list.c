#include "ast/init_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

struct init_declarator_list create_init_declarator_list(struct init_declarator* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }

    struct init_declarator_list res;
    
    res.len = len;
    res.decls = decls;
    return res;
}

void free_init_declarator_list(struct init_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_init_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

