#include "ast/designator_list.h"

#include <stdlib.h>

#include "util.h"

struct designator_list parse_designator_list(struct parser_state* s) {
    struct designator_list res = {
            .len = 1,
            .designators = xmalloc(sizeof(struct designator))
    };

    if (!parse_designator_inplace(s, &res.designators[0])) {
        free(res.designators);
        return (struct designator_list){.len = 0, .designators = NULL};
    }

    size_t alloc_len = res.len;
    while (s->it->type == LINDEX || s->it->type == DOT) {
        if (res.len == alloc_len) {
            grow_alloc((void**)&res.designators, &alloc_len, sizeof(struct designator));
        }

        if (!parse_designator_inplace(s, &res.designators[res.len])) {
            goto fail;
        }

        ++res.len;
    }
    res.designators = xrealloc(res.designators, res.len * sizeof(struct designator));

    return res;

fail:
    free_designator_list(&res);
    return (struct designator_list) {.len = 0, .designators = NULL};
}

void free_designator_list(struct designator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_designator_children(&l->designators[i]);
    }
    free(l->designators);
}
