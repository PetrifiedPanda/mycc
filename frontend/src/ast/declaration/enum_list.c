#include "frontend/ast/declaration/enum_list.h"

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_enum_list(struct parser_state* s, struct enum_list* res) {
    res->len = 1;
    res->enums = mycc_alloc(sizeof *res->enums);
    if (!parse_enumerator_inplace(s, &res->enums[0])) {
        mycc_free(res->enums);
        return false;
    }

    size_t alloc_len = 1;
    while (s->it->type == COMMA && s->it[1].type == IDENTIFIER) {
        accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->enums, &alloc_len, sizeof *res->enums);
        }

        if (!parse_enumerator_inplace(s, &res->enums[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->enums = mycc_realloc(res->enums, res->len * sizeof *res->enums);

    return res;
fail:
    free_enum_list(res);
    return false;
}

void free_enum_list(struct enum_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_enumerator_children(&l->enums[i]);
    }
    mycc_free(l->enums);
}
