#ifndef INIT_DECLARATOR_H
#define INIT_DECLARATOR_H

typedef struct Declarator Declarator;
typedef struct Initializer Initializer;

typedef struct InitDeclarator {
    Declarator* decl;
    Initializer* init;
} InitDeclarator;

InitDeclarator* create_init_declarator(Declarator* decl, Initializer* init);

void free_init_declarator_children(InitDeclarator* d);

#include "ast/declarator.h"
#include "ast/initializer.h"

#endif
