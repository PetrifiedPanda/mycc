#include "frontend/ast/TranslationUnit.h"

#include "util/mem.h"

#include "frontend/ast/declaration/ExternalDeclaration.h"

static void TranslationUnit_free_decls(TranslationUnit* tl) {
    for (uint32_t i = 0; i < tl->len; ++i) {
        ExternalDeclaration_free_children(&tl->external_decls[i]);
    }
    mycc_free(tl->external_decls);
}

TranslationUnit parse_translation_unit(ParserState* s) {
    TranslationUnit res;
    res.tokens = s->_arr;
    uint32_t alloc_num = 1;
    res.len = 0;
    res.external_decls = mycc_alloc(sizeof *res.external_decls * alloc_num);

    while (ParserState_curr_kind(s) != TOKEN_INVALID) {
        if (res.len == alloc_num) {
            mycc_grow_alloc((void**)&res.external_decls,
                            &alloc_num,
                            sizeof *res.external_decls);
        }

        if (!parse_external_declaration_inplace(s,
                                                &res.external_decls[res.len])) {
            TranslationUnit_free_decls(&res);
            res.len = 0;
            res.external_decls = NULL;
            return res;
        }

        ++res.len;
    }

    res.external_decls = mycc_realloc(res.external_decls,
                                      sizeof *res.external_decls * res.len);

    return res;
}

static void TranslationUnit_free_children(TranslationUnit* u) {
    TokenArr_free(&u->tokens);
    TranslationUnit_free_decls(u);
}

void TranslationUnit_free(TranslationUnit* u) {
    TranslationUnit_free_children(u);
}

