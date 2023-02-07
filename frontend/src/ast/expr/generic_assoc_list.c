#include "frontend/ast/expr/generic_assoc_list.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_generic_assoc_list(struct parser_state* s,
                              struct generic_assoc_list* res) {
    size_t alloc_len = 1;
    *res = (struct generic_assoc_list){
        .info = create_ast_node_info(s->it->loc),
        .len = 1,
        .assocs = mycc_alloc(sizeof *res->assocs * alloc_len),
    };

    if (!parse_generic_assoc_inplace(s, &res->assocs[0])) {
        mycc_free(res->assocs);
        return false;
    }

    while (s->it->type == COMMA) {
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->assocs, &alloc_len, sizeof *res->assocs);
        }

        if (!parse_generic_assoc_inplace(s, &res->assocs[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    res->assocs = mycc_realloc(res->assocs, sizeof *res->assocs * res->len);
    return res;

fail:
    free_generic_assoc_list(res);
    return false;
}

void free_generic_assoc_list(struct generic_assoc_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_generic_assoc_children(&l->assocs[i]);
    }
    mycc_free(l->assocs);
}

