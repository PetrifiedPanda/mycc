#include "ast/type_name.h"

#include <stdlib.h>
#include <assert.h>

TypeName* create_type_name(SpecQualList* spec_qual_list, AbstractDeclarator* abstract_decl) {
    assert(spec_qual_list);
    TypeName* res = malloc(sizeof(TypeName));
    if (res) {
        res->spec_qual_list = spec_qual_list;
        res->abstract_decl = abstract_decl;
    }
}

void free_type_name_children(TypeName* n) {
    free_spec_qual_list(n->spec_qual_list);
    if (n->abstract_decl) {
        free_abstract_declarator(n->abstract_decl);
    }
}

void free_type_name(TypeName* n) {
    free_type_name_children(n);
    free(n);
}
