#ifndef MYCC_FRONTEND_AST_TRANSLATION_UNIT_H
#define MYCC_FRONTEND_AST_TRANSLATION_UNIT_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct ExternalDeclaration ExternalDeclaration;

typedef struct {
    size_t len;
    ExternalDeclaration* external_decls;
} TranslationUnit;

TranslationUnit parse_translation_unit(ParserState* s);

void TranslationUnit_free(TranslationUnit* u);

#endif

