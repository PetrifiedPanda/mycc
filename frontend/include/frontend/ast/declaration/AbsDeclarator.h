#ifndef MYCC_FRONTEND_DECLARATION_ABSTRACT_DECL_H
#define MYCC_FRONTEND_DECLARATION_ABSTRACT_DECL_H

#include "frontend/parser/ParserState.h"

typedef struct Pointer Pointer;
typedef struct DirectAbsDeclarator DirectAbsDeclarator;

typedef struct AbsDeclarator {
    Pointer* ptr;
    DirectAbsDeclarator* direct_abs_decl;
} AbsDeclarator;

AbsDeclarator* parse_abs_declarator(ParserState* s);

void AbsDeclarator_free(AbsDeclarator* d);

#endif

