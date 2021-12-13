#include "ast/struct_union_spec.h"

#include <stdlib.h>

#include "util.h"

struct struct_union_spec* create_struct_union_spec(bool is_struct, struct identifier* identifier, struct struct_declaration_list decl_list) {
    struct struct_union_spec* res = xmalloc(sizeof(struct struct_union_spec));
    res->is_struct = is_struct;
    res->identifier = identifier;
    res->decl_list = decl_list;
    
    return res;
}

static void free_children(struct struct_union_spec* s) {
    free_identifier(s->identifier);
    free_struct_declaration_list(&s->decl_list);
}

void free_struct_union_spec(struct struct_union_spec* s) {
    free_children(s);
    free(s);
}

