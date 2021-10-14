#include "ast/func_def.h"

#include <stdlib.h>
#include <assert.h>

FuncDef* create_func_def(DeclarationSpecs* specs, Declarator* decl, DeclarationList decl_list, CompoundStatement* comp) {
    assert(decl);
    assert(comp);
    FuncDef* res = malloc(sizeof(FuncDef));
    if (res) {
        res->specs = specs;
        res->decl = decl;
        res->decl_list = decl_list;
        res->comp = comp;
    }
    return res;
}

static void free_children(FuncDef* d) {
    if (d->specs) {
        free_declaration_specs(d->specs);
    }
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement(d->comp);
}

void free_func_def(FuncDef* d) {
    free_children(d);
    free(d);
}
