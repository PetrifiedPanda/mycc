#include "frontend/ast/TranslationUnit.h"

#include "util/mem.h"

#include "frontend/ast/declaration/ExternalDeclaration.h"

TranslationUnit parse_translation_unit(ParserState* s) {
    TranslationUnit res;
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
            TranslationUnit_free(&res);
            return (TranslationUnit){.len = 0, .external_decls = NULL};
        }

        ++res.len;
    }

    res.external_decls = mycc_realloc(res.external_decls,
                                      sizeof *res.external_decls * res.len);

    return res;
}

static void free_translation_unit_children(TranslationUnit* u) {
    for (size_t i = 0; i < u->len; ++i) {
        ExternalDeclaration_free_children(&u->external_decls[i]);
    }
    mycc_free(u->external_decls);
}

void TranslationUnit_free(TranslationUnit* u) {
    free_translation_unit_children(u);
}

