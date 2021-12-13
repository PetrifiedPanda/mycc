#include "ast/struct_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

struct struct_declarator_list create_struct_declarator_list(struct struct_declarator* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }

    struct struct_declarator_list res;

    res.len = len;
    res.decls = decls;
    return res;
}

void free_struct_declarator_list(struct struct_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}
