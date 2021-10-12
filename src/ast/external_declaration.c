#include "ast/external_declaration.h"

#include <stdlib.h>
#include <assert.h>

ExternalDeclaration* create_external_declaration(Declaration* decl) {
    assert(decl);
    ExternalDeclaration* res = malloc(sizeof(ExternalDeclaration));
    if (res) {
        res->is_func_def = false;
        res->decl = decl;
    }
    return res;
}

ExternalDeclaration* create_external_declaration_func(FuncDef* func_def) {
    assert(func_def);
    ExternalDeclaration* res = malloc(sizeof(ExternalDeclaration));
    if (res) {
        res->is_func_def = true;
        res->func_def = func_def;
    }
    return res;
}

void free_external_declaration_children(ExternalDeclaration* d) {
    if (d->is_func_def) {
        free_func_def(d->func_def);
    } else {
        free_declaration(d->decl);
    }
}
