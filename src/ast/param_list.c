#include "ast/param_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct param_list* create_param_list(struct param_declaration* decls, size_t len) {
    assert(len > 0);
    assert(decls);
    struct param_list* res = xmalloc(sizeof(struct param_list));
    res->len = len;
    res->decls = decls;
    
    return res;
}

static void free_children(struct param_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_param_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

void free_param_list(struct param_list* l) {
    free_children(l);
    free(l);
}

