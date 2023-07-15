#ifndef MYCC_FRONTEND_DECLARATION_FUNC_DEF_H
#define MYCC_FRONTEND_DECLARATION_FUNC_DEF_H

#include "DeclarationSpecs.h"
#include "DeclarationList.h"

#include "frontend/ast/Statement.h"

typedef struct Declarator Declarator;

typedef struct {
    DeclarationSpecs specs;
    Declarator* decl;
    DeclarationList decl_list;
    CompoundStatement comp;
} FuncDef;

void FuncDef_free_children(FuncDef* d);

#endif

