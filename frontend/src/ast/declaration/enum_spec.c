#include "frontend/ast/declaration/enum_spec.h"

#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

static bool parse_enumerator_inplace(struct parser_state* s,
                                     struct enumerator* res) {
    assert(res);

    if (s->it->kind != TOKEN_IDENTIFIER) {
        expected_token_error(s, TOKEN_IDENTIFIER);
        return false;
    }

    struct token* id_token = s->it;
    parser_accept_it(s);

    if (!parser_register_enum_constant(s, id_token)) {
        return false;
    }

    const struct str spell = token_take_spelling(id_token);
    struct source_loc loc = id_token->loc;

    struct const_expr* enum_val = NULL;
    if (s->it->kind == TOKEN_ASSIGN) {
        parser_accept_it(s);
        enum_val = parse_const_expr(s);
        if (!enum_val) {
            free_str(&spell);
            return false;
        }
    }

    res->identifier = create_identifier(&spell, loc);
    res->enum_val = enum_val;

    return true;
}

void free_enumerator_children(struct enumerator* e) {
    free_identifier(e->identifier);
    if (e->enum_val) {
        free_const_expr(e->enum_val);
    }
}

static bool parse_enum_list(struct parser_state* s, struct enum_list* res) {
    res->len = 1;
    res->enums = mycc_alloc(sizeof *res->enums);
    if (!parse_enumerator_inplace(s, &res->enums[0])) {
        mycc_free(res->enums);
        return false;
    }

    size_t alloc_len = 1;
    while (s->it->kind == TOKEN_COMMA && s->it[1].kind == TOKEN_IDENTIFIER) {
        parser_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->enums,
                            &alloc_len,
                            sizeof *res->enums);
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
    if (!parser_accept(s, TOKEN_ENUM)) {
        return NULL;
    }

    struct identifier* id = NULL;
    if (s->it->kind == TOKEN_IDENTIFIER) {
        const struct str spell = token_take_spelling(s->it);
        const struct source_loc id_loc = s->it->loc;
        parser_accept_it(s);
        id = create_identifier(&spell, id_loc);
    }

    struct enum_list enums = {.len = 0, .enums = NULL};
    if (s->it->kind == TOKEN_LBRACE) {
        parser_accept_it(s);
        if (!parse_enum_list(s, &enums)) {
            goto fail;
        }
        assert(enums.len > 0);

        if (s->it->kind == TOKEN_COMMA) {
            parser_accept_it(s);
        }
        if (!parser_accept(s, TOKEN_RBRACE)) {
            free_enum_list(&enums);
            goto fail;
        }
    } else if (id == NULL) {
        expected_token_error(s, TOKEN_LBRACE);
        goto fail;
    }

    return create_enum_spec(loc, id, enums);
fail:
    if (id) {
        free_identifier(id);
    }
    return NULL;
}

static void free_enum_spec_children(struct enum_spec* s) {
    if (s->identifier) {
        free_identifier(s->identifier);
    }
    free_enum_list(&s->enum_list);
}

void free_enum_spec(struct enum_spec* s) {
    free_enum_spec_children(s);
    mycc_free(s);
}

