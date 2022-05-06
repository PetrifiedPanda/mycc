#include "ast/identifier_list.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "parser/parser_util.h"

struct identifier_list parse_identifier_list(struct parser_state* s) {
    if (s->it->type != IDENTIFIER) {
        return (struct identifier_list){.len = 0, .identifiers = NULL};
    }
    struct identifier_list res = {
        .len = 1,
        .identifiers = xmalloc(sizeof(struct identifier_list)),
    };
    char* spell = take_spelling(s->it);
    accept_it(s);
    init_identifier(res.identifiers, spell);

    size_t alloc_len = res.len;
    while (s->it->type == COMMA) {
        accept_it(s);

        if (res.len == alloc_len) {
            grow_alloc((void**)&res.identifiers,
                       &alloc_len,
                       sizeof(struct identifier));
        }

        if (s->it->type != IDENTIFIER) {
            free_identifier_list(&res);
            return (struct identifier_list){.len = 0, .identifiers = NULL};
        }
        spell = take_spelling(s->it);
        accept_it(s);
        init_identifier(&res.identifiers[res.len], spell);

        ++res.len;
    }

    res.identifiers = xrealloc(res.identifiers,
                               sizeof(struct identifier) * res.len);

    return res;
}

void free_identifier_list(struct identifier_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_identifier_children(&l->identifiers[i]);
    }
    free(l->identifiers);
}
