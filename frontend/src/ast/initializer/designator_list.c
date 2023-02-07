#include "frontend/ast/initializer/designator_list.h"

#include "util/mem.h"

bool parse_designator_list(struct parser_state* s, struct designator_list* res) {
    *res = (struct designator_list){
        .len = 1,
        .designators = mycc_alloc(sizeof *res->designators),
    };

    if (!parse_designator_inplace(s, &res->designators[0])) {
        mycc_free(res->designators);
        return false;
    }

    size_t alloc_len = res->len;
    while (s->it->type == LINDEX || s->it->type == DOT) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->designators,
                       &alloc_len,
                       sizeof *res->designators);
        }

        if (!parse_designator_inplace(s, &res->designators[res->len])) {
            goto fail;
        }

        ++res->len;
    }
    res->designators = mycc_realloc(res->designators,
                               res->len * sizeof *res->designators);

    return res;

fail:
    free_designator_list(res);
    return false;
}

void free_designator_list(struct designator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_designator_children(&l->designators[i]);
    }
    mycc_free(l->designators);
}
