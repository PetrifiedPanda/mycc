#include "ast/struct_declaration_list.h"

#include <stdlib.h>
#include <assert.h>

static struct struct_declaration_list create_struct_declaration_list(struct struct_declaration* decls, size_t len) {
    if (len > 0) {
        assert(decls);
    } else {
        assert(decls == NULL);
    }
    return (struct struct_declaration_list){.len = len, .decls = decls};
}

struct struct_declaration_list parse_struct_declaration_list(struct parser_state* s) {
    (void)s;
    return (struct struct_declaration_list){.len = 0, .decls = NULL};
}

void free_struct_declaration_list(struct struct_declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declaration_children(&l->decls[i]);
    }
    free(l->decls);
}

