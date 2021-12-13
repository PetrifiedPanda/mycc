#include "ast/func_def.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct func_def* create_func_def(struct declaration_specs* specs, struct declarator* decl, struct declaration_list decl_list, struct compound_statement* comp) {
    assert(decl);
    assert(comp);
    struct func_def* res = xmalloc(sizeof(struct func_def));
    res->specs = specs;
    res->decl = decl;
    res->decl_list = decl_list;
    res->comp = comp;
    
    return res;
}

void free_func_def_children(struct func_def* d) {
    if (d->specs) {
        free_declaration_specs(d->specs);
    }
    free_declarator(d->decl);
    free_declaration_list(&d->decl_list);
    free_compound_statement(d->comp);
}

void free_func_def(struct func_def* d) {
    free_func_def_children(d);
    free(d);
}

