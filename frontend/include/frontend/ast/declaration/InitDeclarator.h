#ifndef MYCC_FRONTEND_DECLARATION_INIT_DECLARATOR_H
#define MYCC_FRONTEND_DECLARATION_INIT_DECLARATOR_H

#include "frontend/parser/ParserState.h"

typedef struct Declarator Declarator;
typedef struct Initializer Initializer;

typedef struct InitDeclarator {
    Declarator* decl;
    Initializer* init;
} InitDeclarator;

bool parse_init_declarator_typedef_inplace(ParserState* s, InitDeclarator* res);
bool parse_init_declarator_inplace(ParserState* s, InitDeclarator* res);

void InitDeclarator_free_children(InitDeclarator* d);

#endif

