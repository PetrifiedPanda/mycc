#include "ast/external_declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct external_declaration* create_external_declaration(struct declaration decl) {
    struct external_declaration* res = xmalloc(sizeof(struct external_declaration));
    res->is_func_def = false;
    res->decl = decl;
    
    return res;
}

struct external_declaration* create_external_declaration_func(struct func_def func_def) {
    struct external_declaration* res = xmalloc(sizeof(struct external_declaration));
    res->is_func_def = true;
    res->func_def = func_def;
    
    return res;
}

void free_external_declaration_children(struct external_declaration* d) {
    if (d->is_func_def) {
        free_func_def_children(&d->func_def);
    } else {
        free_declaration_children(&d->decl);
    }
}

