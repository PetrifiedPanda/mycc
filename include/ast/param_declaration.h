#ifndef PARAM_DECLARATION_H
#define PARAM_DECLARATION_H

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct Declarator Declarator;
typedef struct AbstractDecl AbstractDecl;

typedef enum {
    PARAM_DECL_DECL,
    PARAM_DECL_ABSTRACT_DECL,
    PARAM_DECL_NONE
} ParamDeclType;

typedef struct ParamDeclaration {
    DeclarationSpecs* decl_specs;
    ParamDeclType type;
    union {
        Declarator* decl;
        AbstractDecl* abstract_decl;
    };
} ParamDeclaration;

ParamDeclaration* create_param_declaration(DeclarationSpecs* decl_specs);
ParamDeclaration* create_param_declaration_declarator(DeclarationSpecs* decl_specs, Declarator* decl);
ParamDeclaration* create_param_declaration_abstract(DeclarationSpecs* decl_specs, AbstractDecl* abstract_decl);

void free_param_declaration_children(ParamDeclaration* d);
void free_param_declaration(ParamDeclaration* d);

#include "ast/declaration_specs.h"
#include "ast/declarator.h"
#include "ast/abstract_decl.h"


#endif