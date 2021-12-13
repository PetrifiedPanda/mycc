#include "ast/type_name.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct type_name* create_type_name(struct spec_qual_list spec_qual_list, struct abstract_declarator* abstract_decl) {
    assert(spec_qual_list.len > 0);
    struct type_name* res = xmalloc(sizeof(struct type_name));
    res->spec_qual_list = spec_qual_list;
    res->abstract_decl = abstract_decl;
    
    return res;
}

void free_type_name_children(struct type_name* n) {
    free_spec_qual_list(&n->spec_qual_list);
    if (n->abstract_decl) {
        free_abstract_declarator(n->abstract_decl);
    }
}

void free_type_name(struct type_name* n) {
    free_type_name_children(n);
    free(n);
}

