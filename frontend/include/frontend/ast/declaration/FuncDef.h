#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "DeclarationList.h"

#include "frontend/ast/Statement.h"

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct Declarator Declarator;

typedef struct {
    DeclarationSpecs* specs;
    Declarator* decl;
    DeclarationList decl_list;
    CompoundStatement comp;
} FuncDef;

void FuncDef_free_children(FuncDef* d);

#endif

