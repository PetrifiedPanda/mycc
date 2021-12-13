#include "ast/declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct declaration* create_declaration(struct declaration_specs* decl_specs, struct init_declarator_list init_decls) {
    assert(decl_specs);
    struct declaration* res = xmalloc(sizeof(struct declaration));
    res->decl_specs = decl_specs;
    res->init_decls = init_decls;
    
    return res;
}

void free_declaration_children(struct declaration* d) {
    free_declaration_specs(d->decl_specs);
    free_init_declarator_list(&d->init_decls);
}

void free_declaration(struct declaration* d) {
    free_declaration_children(d);
    free(d);
}

