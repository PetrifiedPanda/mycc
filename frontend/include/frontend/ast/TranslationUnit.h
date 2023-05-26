#ifndef TRANSLATION_UNIT_H
#define TRANSLATION_UNIT_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct ExternalDeclaration ExternalDeclaration;

typedef struct {
    size_t len;
    ExternalDeclaration* external_decls;
} TranslationUnit;

TranslationUnit parse_translation_unit(ParserState* s);

void TranslationUnit_free(TranslationUnit* u);

#include "frontend/ast/declaration/ExternalDeclaration.h"

#endif

