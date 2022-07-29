#include "frontend/ast/declaration/struct_declaration_list.h"

#include <stdlib.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

struct struct_declaration_list parse_struct_declaration_list(
    struct parser_state* s) {
    struct struct_declaration_list res = {
        .len = 1,
        .decls = xmalloc(sizeof(struct struct_declaration)),
    };

    if (!parse_struct_declaration_inplace(s, &res.decls[0])) {
        free(res.decls);
        return (struct struct_declaration_list){.len = 0, .decls = NULL};
    }

    size_t alloc_len = res.len;
    while (is_declaration(s) || s->it->type == STATIC_ASSERT) {
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.decls,
                       &alloc_len,
                       sizeof(struct struct_declaration));
        }

        if (!parse_struct_declaration_inplace(s, &res.decls[res.len])) {
            free_struct_declaration_list(&res);
            return (struct struct_declaration_list){.len = 0, .decls = NULL};
        }

        ++res.len;
    }

    res.decls = xrealloc(res.decls,
                         sizeof(struct struct_declaration) * res.len);

    return res;
}

void free_struct_declaration_list(struct struct_declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

