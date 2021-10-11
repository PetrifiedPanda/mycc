#ifndef ABSTRACT_DECL_H
#define ABSTRACT_DECL_H

typedef struct Pointer Pointer;
typedef struct DirectAbstractDeclarator DirectAbstractDeclarator;

typedef struct AbstractDeclarator {
    Pointer* ptr;
    DirectAbstractDeclarator* direct_abs_decl;
} AbstractDeclarator;

void free_abstract_declarator(AbstractDeclarator* d);

#include "ast/pointer.h"
#include "ast/direct_abstract_declarator.h"

#endif
