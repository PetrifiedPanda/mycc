#ifndef MYCC_FRONTEND_DECLARATION_IDENTIFIER_LIST_H
#define MYCC_FRONTEND_DECLARATION_IDENTIFIER_LIST_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct Identifier Identifier;

typedef struct {
    uint32_t len;
    Identifier* identifiers;
} IdentifierList;

bool parse_identifier_list(ParserState* s, IdentifierList* res);

void IdentifierList_free(IdentifierList* l);

#endif
