#include "ast/init_declarator_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct init_declarator_list parse_init_declarator_list_first(struct parser_state* s, struct init_declarator* first_decl) {
    assert(first_decl);
    struct init_declarator_list res = {
        .len = 1,
        .decls = first_decl
    };

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.decls, &alloc_len, sizeof(struct init_declarator));
        }

        if (!parse_init_declarator_inplace(s, &res.decls[res.len])) {
            free_init_declarator_list(&res);
            return (struct init_declarator_list) {
                .len = 0,
                .decls = NULL
            };
        }

        ++res.len;
    }

    res.decls = xrealloc(res.decls, sizeof(struct init_declarator) * res.len);

    return res;
}

struct init_declarator_list parse_init_declarator_list(struct parser_state* s) {
    struct init_declarator* first_decl = xmalloc(sizeof(struct init_declarator));

    if (!parse_init_declarator_inplace(s, first_decl)) {
        free(first_decl);
        return (struct init_declarator_list) {
            .len = 0,
            .decls = NULL
        };
    }

    return parse_init_declarator_list_first(s, first_decl);
}

void free_init_declarator_list(struct init_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_init_declarator_children(&l->decls[i]);
    }
    free(l->decls);
}

