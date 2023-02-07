#include "frontend/ast/statement/compound_statement.h"

#include <assert.h>

#include "util/mem.h"

struct compound_statement* parse_compound_statement(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, LBRACE)) {
        return NULL;
    }

    parser_push_scope(s);

    struct compound_statement* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->items = NULL;
    res->len = 0;

    size_t alloc_len = res->len;
    while (s->it->type != RBRACE) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->items,
                       &alloc_len,
                       sizeof *res->items);
        }

        if (!parse_block_item_inplace(s, &res->items[res->len])) {
            free_compound_statement(res);
            return NULL;
        }

        ++res->len;
    }

    res->items = mycc_realloc(res->items, sizeof *res->items * res->len);

    assert(s->it->type == RBRACE);
    accept_it(s);

    parser_pop_scope(s);

    return res;
}

void free_children(struct compound_statement* s) {
    for (size_t i = 0; i < s->len; ++i) {
        free_block_item_children(&s->items[i]);
    }
    mycc_free(s->items);
}

void free_compound_statement(struct compound_statement* s) {
    free_children(s);
    mycc_free(s);
}
