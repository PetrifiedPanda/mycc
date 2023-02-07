#include "frontend/ast/declaration/struct_declarator_list.h"

#include "util/mem.h"

bool parse_struct_declarator_list(struct parser_state* s,
                                  struct struct_declarator_list* res) {
    *res = (struct struct_declarator_list){
        .len = 1,
        .decls = mycc_alloc(sizeof *res->decls),
    };

    if (!parse_struct_declarator_inplace(s, &res->decls[0])) {
        mycc_free(res->decls);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->type == COMMA) {
        accept_it(s);
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->decls, &alloc_len, sizeof *res->decls);
        }

        if (!parse_struct_declarator_inplace(s, &res->decls[res->len])) {
            free_struct_declarator_list(res);
            return false;
        }

        ++res->len;
    }

    res->decls = mycc_realloc(res->decls, sizeof *res->decls * res->len);

    return res;
}

void free_struct_declarator_list(struct struct_declarator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_struct_declarator_children(&l->decls[i]);
    }
    mycc_free(l->decls);
}

