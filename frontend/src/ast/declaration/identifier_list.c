#include "frontend/ast/declaration/identifier_list.h"

#include <stdlib.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

bool parse_identifier_list(struct parser_state* s,
                           struct identifier_list* res) {
    if (s->it->type != IDENTIFIER) {
        return false;
    }
    *res = (struct identifier_list){
        .len = 1,
        .identifiers = xmalloc(sizeof *res->identifiers),
    };
    struct str spell = take_spelling(s->it);
    struct source_loc loc = s->it->loc;
    accept_it(s);
    init_identifier(res->identifiers, &spell, loc);

    size_t alloc_len = res->len;
    while (s->it->type == COMMA) {
        accept_it(s);

        if (res->len == alloc_len) {
            grow_alloc((void**)&res->identifiers,
                       &alloc_len,
                       sizeof *res->identifiers);
        }

        if (s->it->type != IDENTIFIER) {
            free_identifier_list(res);
            return false;
        }
        spell = take_spelling(s->it);
        loc = s->it->loc;
        accept_it(s);
        init_identifier(&res->identifiers[res->len], &spell, loc);

        ++res->len;
    }

    res->identifiers = xrealloc(res->identifiers,
                                sizeof *res->identifiers * res->len);

    return res;
}

void free_identifier_list(struct identifier_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_identifier_children(&l->identifiers[i]);
    }
    free(l->identifiers);
}
