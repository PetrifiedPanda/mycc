#ifndef TRANSLATION_UNIT_H
#define TRANSLATION_UNIT_H

#include <stddef.h>

#include "parser/parser_state.h"

struct external_declaration;

struct translation_unit {
    size_t len;
    struct external_declaration* external_decls;
};

struct translation_unit parse_translation_unit(struct parser_state* s);

void free_translation_unit(struct translation_unit* u);

#include "ast/declaration/external_declaration.h"

#endif

