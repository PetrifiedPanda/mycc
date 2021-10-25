#include "ast/param_declaration.h"

#include <stdlib.h>
#include <assert.h>

ParamDeclaration* create_param_declaration(DeclarationSpecs* decl_specs) {
    assert(decl_specs);
    ParamDeclaration* res = malloc(sizeof(ParamDeclaration));
    if (res) {
        res->type = PARAM_DECL_NONE;
        res->decl_specs = decl_specs;
    }
    return res;
}

ParamDeclaration* create_param_declaration_declarator(DeclarationSpecs* decl_specs, Declarator* decl) {
    ParamDeclaration* res = create_param_declaration(decl_specs);
    if (res) {
        res->type = PARAM_DECL_DECL;
        res->decl = decl;
    }
    return res;
}

ParamDeclaration* create_param_declaration_abstract(DeclarationSpecs* decl_specs, AbstractDeclarator* abstract_decl) {
    ParamDeclaration* res = create_param_declaration(decl_specs);
    if (res) {
        res->type = PARAM_DECL_ABSTRACT_DECL;
        res->abstract_decl = abstract_decl;
    }
    return res;
}

void free_param_declaration_children(ParamDeclaration* d) {
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

void free_param_declaration(ParamDeclaration* d) {
    free_param_declaration_children(d);
    free(d);
}
