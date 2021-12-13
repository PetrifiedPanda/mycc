#include "ast/param_declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct param_declaration* create_param_declaration(struct declaration_specs* decl_specs) {
    assert(decl_specs);
    struct param_declaration* res = xmalloc(sizeof(struct param_declaration));
    res->type = PARAM_DECL_NONE;
    res->decl_specs = decl_specs;
    
    return res;
}

struct param_declaration* create_param_declaration_declarator(struct declaration_specs* decl_specs, struct declarator* decl) {
    struct param_declaration* res = create_param_declaration(decl_specs);
    res->type = PARAM_DECL_DECL;
    res->decl = decl;
    
    return res;
}

struct param_declaration* create_param_declaration_abstract(struct declaration_specs* decl_specs, struct abstract_declarator* abstract_decl) {
    struct param_declaration* res = create_param_declaration(decl_specs);
    res->type = PARAM_DECL_ABSTRACT_DECL;
    res->abstract_decl = abstract_decl;
    
    return res;
}

void free_param_declaration_children(struct param_declaration* d) {
    free_declaration_specs(d->decl_specs);
    switch (d->type) {
    case PARAM_DECL_DECL:
        free_declarator(d->decl);
        break;
    case PARAM_DECL_ABSTRACT_DECL:
        free_abstract_declarator(d->abstract_decl);
        break;
    case PARAM_DECL_NONE:
        break;
    }
}

void free_param_declaration(struct param_declaration* d) {
    free_param_declaration_children(d);
    free(d);
}

