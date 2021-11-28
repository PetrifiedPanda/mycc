#include "ast/declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

Declaration* create_declaration(DeclarationSpecs* decl_specs, InitDeclaratorList init_decls) {
    assert(decl_specs);
    Declaration* res = xmalloc(sizeof(Declaration));
    res->decl_specs = decl_specs;
    res->init_decls = init_decls;
    
    return res;
}

void free_declaration_children(Declaration* d) {
    free_declaration_specs(d->decl_specs);
    free_init_declarator_list(&d->init_decls);
}

void free_declaration(Declaration* d) {
    free_declaration_children(d);
    free(d);
}

