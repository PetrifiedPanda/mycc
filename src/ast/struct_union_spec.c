#include "ast/struct_union_spec.h"

#include <stdlib.h>

#include "util.h"

StructUnionSpec* create_struct_union_spec(bool is_struct, char* identifier, StructDeclarationList decl_list) {
    StructUnionSpec* res = xmalloc(sizeof(StructUnionSpec));
    res->is_struct = is_struct;
    res->identifier = identifier;
    res->decl_list = decl_list;
    
    return res;
}

static void free_children(StructUnionSpec* s) {
    free(s->identifier);
    free_struct_declaration_list(&s->decl_list);
}

void free_struct_union_spec(StructUnionSpec* s) {
    free_children(s);
    free(s);
}

