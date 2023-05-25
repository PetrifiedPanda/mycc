#ifndef DECLARATOR_H
#define DECLARATOR_H

#include "frontend/parser/ParserState.h"

typedef struct Pointer Pointer;
typedef struct DirectDeclarator DirectDeclarator;

typedef struct Declarator {
    Pointer* ptr;
    DirectDeclarator* direct_decl;
} Declarator;

Declarator* parse_declarator(ParserState* s);
Declarator* parse_declarator_typedef(ParserState* s);

void free_declarator(Declarator* d);

#include "Pointer.h"
#include "DirectDeclarator.h"

#endif

