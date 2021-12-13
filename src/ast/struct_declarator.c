#include "ast/struct_declarator.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct struct_declarator* create_struct_declarator(struct declarator* decl, struct const_expr* bit_field) {
    assert(decl || bit_field);
    struct struct_declarator* res = xmalloc(sizeof(struct struct_declarator));
    res->decl = decl;
    res->bit_field = bit_field;
    
    return res;
}

void free_struct_declarator_children(struct struct_declarator* d) {
    if (d->decl) {
        free_declarator(d->decl);
    }
    if (d->bit_field) {
        free_const_expr(d->bit_field);
    }
}

void free_struct_declarator(struct struct_declarator* d) {
    free_struct_declarator_children(d);
    free(d);
}

