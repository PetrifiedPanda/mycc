#ifndef FUNC_DEF_H
#define FUNC_DEF_H

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct Declarator Declarator;
typedef struct DeclarationList DeclarationList;
typedef struct CompoundStatement CompoundStatement;

typedef struct FuncDef {
    DeclarationSpecs* specs;
    Declarator* decl;
    DeclarationList* decl_list;
    CompoundStatement* comp;
} FuncDef;

FuncDef* create_func_def(DeclarationSpecs* specs, Declarator* decl, DeclarationList* decl_list, CompoundStatement* comp);

void free_func_def(FuncDef* d);

#include "ast/declaration_specs.h"
#include "ast/declarator.h"
#include "ast/declaration_list.h"
#include "ast/compound_statement.h"

#endif
