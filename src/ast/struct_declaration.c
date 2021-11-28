#include "ast/struct_declaration.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

StructDeclaration* create_struct_declaration(SpecQualList spec_qual_list, StructDeclaratorList decls) {
    assert(spec_qual_list.len > 0);
    assert(decls.len > 0);
    StructDeclaration* res = xmalloc(sizeof(StructDeclaration));
    res->spec_qual_list = spec_qual_list;
    res->decls = decls;
    
    return res;
}

void free_struct_declaration_children(StructDeclaration* d) {
    free_spec_qual_list(&d->spec_qual_list);
    free_struct_declarator_list(&d->decls);
}

void free_struct_declaration(StructDeclaration* d) {
    free_struct_declaration_children(d);
    free(d);
}

