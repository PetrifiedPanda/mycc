#ifndef MYCC_FRONTEND_DECLARATION_DECLARATION_LIST_H
#define MYCC_FRONTEND_DECLARATION_DECLARATION_LIST_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct Declaration Declaration;

typedef struct {
    size_t len;
    Declaration* decls;
} DeclarationList;

bool parse_declaration_list(ParserState* s, DeclarationList* res);

void DeclarationList_free(DeclarationList* l);

#endif

