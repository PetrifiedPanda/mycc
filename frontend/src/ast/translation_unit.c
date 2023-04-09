#include "frontend/ast/translation_unit.h"

#include "util/mem.h"

struct translation_unit parse_translation_unit(struct parser_state* s) {
    struct translation_unit res;
    size_t alloc_num = 1;
    res.len = 0;
    res.external_decls = mycc_alloc(sizeof *res.external_decls * alloc_num);

    while (s->it->kind != TOKEN_INVALID) {
        if (res.len == alloc_num) {
            mycc_grow_alloc((void**)&res.external_decls,
                            &alloc_num,
                            sizeof *res.external_decls);
        }

        if (!parse_external_declaration_inplace(s,
                                                &res.external_decls[res.len])) {
            free_translation_unit(&res);
            return (struct translation_unit){.len = 0, .external_decls = NULL};
        }

        ++res.len;
    }

    res.external_decls = mycc_realloc(res.external_decls,
                                      sizeof *res.external_decls * res.len);

    return res;
}

static void free_translation_unit_children(struct translation_unit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        free_external_declaration_children(&u->external_decls[i]);
    }
    mycc_free(u->external_decls);
}

void free_translation_unit(struct translation_unit* u) {
    free_translation_unit_children(u);
}

