#include "ast/declaration/enum_spec.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"

#include "parser/parser_util.h"

static struct enum_spec* create_enum_spec(struct identifier* identifier,
                                          struct enum_list enum_list) {
    assert(identifier || enum_list.len > 0);
    struct enum_spec* res = xmalloc(sizeof(struct enum_spec));
    res->identifier = identifier;
    res->enum_list = enum_list;

    return res;
}

struct enum_spec* parse_enum_spec(struct parser_state* s) {
    if (!accept(s, ENUM)) {
        return NULL;
    }

    struct identifier* id = NULL;
    if (s->it->type == IDENTIFIER) {
        char* spell = take_spelling(s->it);
        accept_it(s);
        id = create_identifier(spell);
    }

    struct enum_list enums = {.len = 0, .enums = NULL};
    if (s->it->type == LBRACE) {
        accept_it(s);
        enums = parse_enum_list(s);
        if (enums.len == 0) {
            goto fail;
        }
        if (s->it->type == COMMA) {
            accept_it(s);
        }
        if (!accept(s, RBRACE)) {
            free_enum_list(&enums);
            goto fail;
        }
    } else if (id == NULL) {
        expected_token_error(LBRACE, s->it);
        goto fail;
    }

    return create_enum_spec(id, enums);
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
    free(s);
}
