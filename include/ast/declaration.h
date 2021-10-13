#ifndef DECLARATION_H
#define DECLARATION_H

typedef struct Pointer Pointer;
typedef struct DirectDeclarator DirectDeclarator;

typedef struct Declaration {
    Pointer* ptr;
    DirectDeclarator* direct_decl;
} Declaration;

Declaration* create_declaration(Pointer* ptr, DirectDeclarator* direct_decl);

void free_declaration_children(Declaration* d);
void free_declaration(Declaration* d);

#include "ast/pointer.h"
#include "ast/direct_declarator.h"

#endif
