#include "ast/struct_declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct struct_declaration* create_struct_declaration(struct spec_qual_list spec_qual_list, struct struct_declarator_list decls) {
    assert(spec_qual_list.len > 0);
    assert(decls.len > 0);
    struct struct_declaration* res = xmalloc(sizeof(struct struct_declaration));
    res->is_static_assert = false;
    res->spec_qual_list = spec_qual_list;
    res->decls = decls;
    
    return res;
}

struct struct_declaration* create_struct_declaration_assert(struct static_assert_declaration* assert_decl) {
    assert(assert_decl);
    struct struct_declaration* res = xmalloc(sizeof(struct struct_declaration));
    res->is_static_assert = true;
    res->assert = assert_decl;

    return res;
}

void free_struct_declaration_children(struct struct_declaration* d) {
    free_spec_qual_list(&d->spec_qual_list);
    free_struct_declarator_list(&d->decls);
}

void free_struct_declaration(struct struct_declaration* d) {
    free_struct_declaration_children(d);
    free(d);
}

