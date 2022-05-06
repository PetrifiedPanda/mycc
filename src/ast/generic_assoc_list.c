#include "ast/generic_assoc_list.h"

#include "util/mem.h"

#include "parser/parser_util.h"

struct generic_assoc_list parse_generic_assoc_list(struct parser_state* s) {
    size_t alloc_len = 1;
    struct generic_assoc_list res = {
        .len = 1,
        .assocs = xmalloc(sizeof(struct generic_assoc) * alloc_len),
    };

    if (!parse_generic_assoc_inplace(s, &res.assocs[0])) {
        free(res.assocs);
        return (struct generic_assoc_list){.len = 0, .assocs = NULL};
    }

    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.assocs,
                       &alloc_len,
                       sizeof(struct generic_assoc));
        }

        if (!parse_generic_assoc_inplace(s, &res.assocs[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    res.assocs = xrealloc(res.assocs, res.len * sizeof(struct generic_assoc));

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
