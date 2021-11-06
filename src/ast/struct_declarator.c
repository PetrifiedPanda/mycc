#include "ast/struct_declarator.h"

#include <stdlib.h>
#include <assert.h>

StructDeclarator* create_struct_declarator(Declarator* decl, ConstExpr* bit_field) {
    assert(decl || bit_field);
    StructDeclarator* res = malloc(sizeof(StructDeclarator));
    if (res) {
        res->decl = decl;
        res->bit_field = bit_field;
    }
    return res;
}

void free_struct_declarator_children(StructDeclarator* d) {
    if (d->decl) {
        free_declarator(d->decl);
    }
    if (d->bit_field) {
        free_const_expr(d->bit_field);
    }
}

void free_struct_declarator(StructDeclarator* d) {
    free_struct_declarator_children(d);
    free(d);
}

