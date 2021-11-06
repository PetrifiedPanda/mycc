#ifndef DECLARATOR_H
#define DECLARATOR_H

typedef struct Pointer Pointer;
typedef struct DirectDeclarator DirectDeclarator;

typedef struct Declarator {
    Pointer* ptr;
    DirectDeclarator* direct_decl;
} Declarator;

Declarator* create_declarator(Pointer* ptr, DirectDeclarator* direct_decl);

void free_declarator(Declarator* d);

#include "ast/pointer.h"
#include "ast/direct_declarator.h"

#endif

