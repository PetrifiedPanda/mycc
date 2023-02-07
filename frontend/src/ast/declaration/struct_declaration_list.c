#include "frontend/ast/declaration/struct_declaration_list.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_struct_declaration_list(struct parser_state* s,
                                   struct struct_declaration_list* res) {
    *res = (struct struct_declaration_list){
        .len = 1,
        .decls = mycc_alloc(sizeof *res->decls),
    };

    if (!parse_struct_declaration_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (is_declaration(s) || s->it->type == STATIC_ASSERT) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls, &alloc_len, sizeof *res->decls);
        }

        if (!parse_struct_declaration_inplace(s, &res->decls[res->len])) {
            free_struct_declaration_list(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return res;
}

void free_struct_declaration_list(struct struct_declaration_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declaration_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

