#ifndef IDENTIFIER_LIST_H
#define IDENTIFIER_LIST_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct Identifier Identifier;

typedef struct {
    size_t len;
    Identifier* identifiers;
} IdentifierList;

bool parse_identifier_list(ParserState* s, IdentifierList* res);

void IdentifierList_free(IdentifierList* l);

#include "frontend/ast/Identifier.h"

#endif
