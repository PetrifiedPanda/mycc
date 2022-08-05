#include "frontend/ast/translation_unit.h"

#include <stdlib.h>

#include "util/mem.h"

struct translation_unit parse_translation_unit(struct parser_state* s) {
    struct translation_unit res;
    size_t alloc_num = 1;
    res.len = 0;
    res.external_decls = xmalloc(sizeof(struct external_declaration)
                                 * alloc_num);

    while (s->it->type != INVALID) {
        if (res.len == alloc_num) {
            grow_alloc((void**)&res.external_decls,
                       &alloc_num,
                       sizeof(struct external_declaration));
        }

        if (!parse_external_declaration_inplace(s,
                                                &res.external_decls[res.len])) {
            free_translation_unit(&res);
            return (struct translation_unit){.len = 0, .external_decls = NULL};
        }

        ++res.len;
    }

    if (res.len != alloc_num) {
        res.external_decls = xrealloc(
            res.external_decls,
            res.len * sizeof(struct external_declaration));
    }

    return res;
}

static void free_children(struct translation_unit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        free_external_declaration_children(&u->external_decls[i]);
    }
    free(u->external_decls);
}

void free_translation_unit(struct translation_unit* u) {
    free_children(u);
}
