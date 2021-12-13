#ifndef TRANSLATION_UNIT_H
#define TRANSLATION_UNIT_H

#include <stddef.h>

struct external_declaration;

struct translation_unit {
    size_t len;
    struct external_declaration* external_decls;
};

struct translation_unit* create_translation_unit(struct external_declaration* external_decls, size_t len);

void free_translation_unit(struct translation_unit* u);

#include "ast/external_declaration.h"

#endif

