#ifndef DECLARATION_LIST_H
#define DECLARATION_LIST_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct Declaration Declaration;

typedef struct {
    size_t len;
    Declaration* decls;
} DeclarationList;

bool parse_declaration_list(ParserState* s, DeclarationList* res);

void free_declaration_list(DeclarationList* l);

#include "Declaration.h"

#endif

