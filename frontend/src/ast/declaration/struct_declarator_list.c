#include "frontend/ast/declaration/struct_declarator_list.h"

#include <stdlib.h>

#include "util/mem.h"

struct struct_declarator_list parse_struct_declarator_list(
    struct parser_state* s) {
    struct struct_declarator_list res = {
        .len = 1,
        .decls = xmalloc(sizeof *res.decls),
    };

    if (!parse_struct_declarator_inplace(s, &res.decls[0])) {
        free(res.decls);
        return (struct struct_declarator_list){.len = 0, .decls = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.decls,
                       &alloc_len,
                       sizeof *res.decls);
        }

        if (!parse_struct_declarator_inplace(s, &res.decls[res.len])) {
            free_struct_declarator_list(&res);
            return (struct struct_declarator_list){.len = 0, .decls = NULL};
        }

        ++res.len;
    }

    res.decls = xrealloc(res.decls, sizeof *res.decls * res.len);

    return res;
}

void free_struct_declarator_list(struct struct_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}
