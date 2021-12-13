#include "ast/declaration_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct declaration_list* create_declaration_list(struct declaration* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }
    struct declaration_list* res = xmalloc(sizeof(struct declaration_list));
    res->len = len;
    res->decls = decls;

    return res;
}

void free_declaration_list(struct declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}
