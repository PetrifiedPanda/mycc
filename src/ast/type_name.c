#include "ast/type_name.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static struct type_name* create_type_name(struct spec_qual_list spec_qual_list, struct abs_declarator* abstract_decl) {
    assert(spec_qual_list.len > 0);
    struct type_name* res = xmalloc(sizeof(struct type_name));
    res->spec_qual_list = spec_qual_list;
    res->abstract_decl = abstract_decl;
    
    return res;
}

bool parse_type_name_inplace(struct parser_state* s, struct type_name* res) {
    assert(res);
    (void)s;
    (void)res;
    // TODO:
    return false;
}

struct type_name* parse_type_name(struct parser_state* s) {
    struct type_name* res = xmalloc(sizeof(struct type_name));
    if (!parse_type_name_inplace(s, res)) {
        free(res);
        return NULL;
    }
    return res;
}

void free_type_name_children(struct type_name* n) {
    free_spec_qual_list(&n->spec_qual_list);
    if (n->abstract_decl) {
        free_abs_declarator(n->abstract_decl);
    }
}

void free_type_name(struct type_name* n) {
    free_type_name_children(n);
    free(n);
}

