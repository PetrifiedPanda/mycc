#include "ast/param_list.h"

#include <stdlib.h>

#include "util.h"

struct param_list* parse_param_list(struct parser_state* s) {
    struct param_list* res = xmalloc(sizeof(struct param_list));
    res->decls = xmalloc(sizeof(struct param_declaration));
    res->len = 1;

    if (!parse_param_declaration_inplace(s, &res->decls[0])) {
        free(res->decls);
        free(res);
        return NULL;
    }

    size_t alloc_len = res->len;
    while (s->it->type == COMMA && s->it[1].type != ELLIPSIS) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->decls,
                       &alloc_len,
                       sizeof(struct param_declaration));
        }

        if (!parse_param_declaration_inplace(s, &res->decls[res->len])) {
            free_param_list(res);
            return NULL;
        }

        ++res->len;
    }

    res->decls = xrealloc(res->decls,
                          sizeof(struct param_declaration) * res->len);

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
