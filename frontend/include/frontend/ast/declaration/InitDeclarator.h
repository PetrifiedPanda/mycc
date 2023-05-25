#ifndef INIT_DECLARATOR_H
#define INIT_DECLARATOR_H

#include "frontend/parser/ParserState.h"

typedef struct Declarator Declarator;
typedef struct Initializer Initializer;

typedef struct InitDeclarator {
    Declarator* decl;
    Initializer* init;
} InitDeclarator;

bool parse_init_declarator_typedef_inplace(ParserState* s, InitDeclarator* res);
bool parse_init_declarator_inplace(ParserState* s, InitDeclarator* res);

void free_init_declarator_children(InitDeclarator* d);

#include "frontend/ast/declaration/Declarator.h"

#include "frontend/ast/Initializer.h"

#endif

