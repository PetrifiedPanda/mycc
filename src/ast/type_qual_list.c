#include "ast/type_qual_list.h"

#include <stdlib.h>

#include "util.h"

struct type_qual_list parse_type_qual_list(struct parser_state* s) {
    struct type_qual_list res = {
            .type_quals = xmalloc(sizeof(struct type_qual)),
            .len = 0
    };

    size_t alloc_size = 1;

    while (is_type_qual(s->it->type)) {
        if (res.len == alloc_size) {
            grow_alloc((void**)&res.type_quals, &alloc_size, sizeof(struct type_qual));
        }
        res.type_quals[res.len] = parse_type_qual(s);
        if (res.type_quals[res.len].type == INVALID) {
            return (struct type_qual_list){
                .type_quals = NULL,
                .len = 0
            };
        }

        ++res.len;
    }

    res.type_quals = xrealloc(res.type_quals, res.len * sizeof(struct type_qual));
    return res;
}

void free_type_qual_list(struct type_qual_list* l) {
    free(l->type_quals);
}

