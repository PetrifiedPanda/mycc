#include "frontend/ast/declaration/enum_spec.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static struct enum_spec* create_enum_spec(struct source_loc loc,
                                          struct identifier* identifier,
                                          struct enum_list enum_list) {
    assert(identifier || enum_list.len > 0);
    struct enum_spec* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->identifier = identifier;
    res->enum_list = enum_list;

    return res;
}

struct enum_spec* parse_enum_spec(struct parser_state* s) {
    const struct source_loc loc = s->it->loc;
    if (!accept(s, ENUM)) {
        return NULL;
    }

    struct identifier* id = NULL;
    if (s->it->type == IDENTIFIER) {
        const struct str spell = take_spelling(s->it);
        const struct source_loc id_loc = s->it->loc;
        accept_it(s);
        id = create_identifier(&spell, id_loc);
    }

    struct enum_list enums = {.len = 0, .enums = NULL};
    if (s->it->type == LBRACE) {
        accept_it(s);
        if (!parse_enum_list(s, &enums)) {
            goto fail;
        }
        assert(enums.len > 0);

        if (s->it->type == COMMA) {
            accept_it(s);
        }
        if (!accept(s, RBRACE)) {
            free_enum_list(&enums);
            goto fail;
        }
    } else if (id == NULL) {
        expected_token_error(s, LBRACE);
        goto fail;
    }

    return create_enum_spec(loc, id, enums);
fail:
    if (id) {
        free_identifier(id);
    }
    return NULL;
}

static void free_children(struct enum_spec* s) {
    if (s->identifier) {
        free_identifier(s->identifier);
    }
    free_enum_list(&s->enum_list);
}

void free_enum_spec(struct enum_spec* s) {
    free_children(s);
    mycc_free(s);
}

