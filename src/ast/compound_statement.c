#include "ast/compound_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct compound_statement* parse_compound_statement(struct parser_state* s) {
    if (!accept(s, LBRACE)) {
        return NULL;
    }

    struct compound_statement* res = xmalloc(sizeof(struct compound_statement));
    res->items = NULL;
    size_t alloc_len = res->len = 0;
    while (s->it->type != RBRACE) {
        if (res->len == alloc_len) {
            grow_alloc((void**)&res->items, &alloc_len, sizeof(struct block_item));
        }

        if (!parse_block_item_inplace(s, &res->items[res->len])) {
            free_compound_statement(res);
            return NULL;
        }

        ++res->len;
    }

    assert(s->it->type == RBRACE);
    accept_it(s);

    return res;
}

void free_children(struct compound_statement* s) {
    for (size_t i = 0; i < s->len; ++i) {
        free_block_item_children(&s->items[i]);
    }
    free(s->items);
}

void free_compound_statement(struct compound_statement* s) {
    free_children(s);
    free(s);
}

