#include "frontend/ast/declaration/declaration_list.h"

#include <stdlib.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_declaration_list(struct parser_state* s,
                            struct declaration_list* res) {
    res->len = 1;
    res->decls = xmalloc(sizeof *res->decls);

    if (!parse_declaration_inplace(s, res->decls)) {
        free(res->decls);
        return false;
    }

    size_t alloc_size = res->len;
    while (is_declaration(s)) {
        if (res->len == alloc_size) {
            grow_alloc((void**)&res->decls, &alloc_size, sizeof *res->decls);
        }

        if (!parse_declaration_inplace(s, &res->decls[res->len])) {
            free_declaration_list(res);
            return false;
        }

        ++res->len;
    }

    return res;
}

void free_declaration_list(struct declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

