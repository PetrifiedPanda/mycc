#ifndef DECLARATION_H
#define DECLARATION_H

typedef struct DeclarationSpecs DeclarationSpecs;
typedef struct InitDeclaratorList InitDeclaratorList;

typedef struct Declaration {
    DeclarationSpecs* decl_specs;
    InitDeclaratorList* init_decls;
} Declaration;

Declaration* create_declaration(DeclarationSpecs* decl_specs, InitDeclaratorList* init_decls);

void free_declaration_children(Declaration* d);
void free_declaration(Declaration* d);

#include "ast/declaration_specs.h"
#include "ast/init_declarator_list.h"

#endif
