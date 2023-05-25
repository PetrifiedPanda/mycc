#ifndef ABSTRACT_DECL_H
#define ABSTRACT_DECL_H

#include "frontend/parser/ParserState.h"

typedef struct Pointer Pointer;
typedef struct DirectAbsDeclarator DirectAbsDeclarator;

typedef struct AbsDeclarator {
    Pointer* ptr;
    DirectAbsDeclarator* direct_abs_decl;
} AbsDeclarator;

AbsDeclarator* parse_abs_declarator(ParserState* s);

void free_abs_declarator(AbsDeclarator* d);

#include "Pointer.h"
#include "DirectAbsDeclarator.h"

#endif

