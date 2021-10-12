#ifndef TRANSLATION_UNIT_H
#define TRANSLATION_UNIT_H

#include <stddef.h>

typedef struct ExternalDeclaration ExternalDeclaration;

typedef struct TranslationUnit {
    size_t len;
    ExternalDeclaration* external_decls;
} TranslationUnit;

TranslationUnit* create_translation_unit(ExternalDeclaration* external_decls, size_t len);

void free_translation_unit(TranslationUnit* u);

#include "ast/external_declaration.h"

#endif
