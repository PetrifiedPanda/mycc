#include "frontend/ast/expr/generic_assoc_list.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

struct generic_assoc_list parse_generic_assoc_list(struct parser_state* s) {
    size_t alloc_len = 1;
    struct generic_assoc_list res = {
        .info = create_ast_node_info(s->it->loc),
        .len = 1,
        .assocs = xmalloc(sizeof *res.assocs * alloc_len),
    };
    
    if (!parse_generic_assoc_inplace(s, &res.assocs[0])) {
        free(res.assocs);
        return (struct generic_assoc_list){.len = 0, .assocs = NULL};
    }

    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.assocs, &alloc_len, sizeof *res.assocs);
        }

        if (!parse_generic_assoc_inplace(s, &res.assocs[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    res.assocs = xrealloc(res.assocs, sizeof *res.assocs * res.len);
    return res;

fail:
    free_generic_assoc_list(&res);
    return (struct generic_assoc_list){.len = 0, .assocs = NULL};
}

void free_generic_assoc_list(struct generic_assoc_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_generic_assoc_children(&l->assocs[i]);
    }
    free(l->assocs);
}

