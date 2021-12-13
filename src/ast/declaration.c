#include "ast/declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct declaration* create_declaration(struct declaration_specs* decl_specs, struct init_declarator_list init_decls) {
    assert(decl_specs);
    struct declaration* res = xmalloc(sizeof(struct declaration));
    res->is_normal_decl = true;
    res->decl_specs = decl_specs;
    res->init_decls = init_decls;
    
    return res;
}

struct declaration* create_declaration_assert(struct static_assert_declaration* static_assert_decl) {
    assert(static_assert_decl);
    struct declaration* res = xmalloc(sizeof(struct declaration));
    res->is_normal_decl = false;
    res->static_assert_decl = static_assert_decl;

    return res;
}

void free_declaration_children(struct declaration* d) {
    if (d->is_normal_decl) {
        free_declaration_specs(d->decl_specs);
        free_init_declarator_list(&d->init_decls);
    } else {
        free_static_assert_declaration(d->static_assert_decl);
    }
}

void free_declaration(struct declaration* d) {
    free_declaration_children(d);
    free(d);
}

