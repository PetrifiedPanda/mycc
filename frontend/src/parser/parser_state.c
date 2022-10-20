#include "frontend/parser/parser_state.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

enum identifier_type {
    ID_TYPE_NONE,
    ID_TYPE_TYPEDEF_NAME,
    ID_TYPE_ENUM_CONSTANT
};

/**
 * The pointers stored in this need to be valid at least as long
 * as the parser_state
 */
struct identifier_type_data {
    struct source_loc loc;
    enum identifier_type type;
};

enum {
    SCOPE_MAP_INIT_CAP = 100
};

static bool register_identifier(struct parser_state* s,
                                const struct token* token,
                                enum identifier_type type);

static enum identifier_type get_item(const struct parser_state* s,
                                     const struct str* spell);

struct parser_state create_parser_state(struct token* tokens,
                                        struct parser_err* err) {
    assert(tokens);

    struct parser_state res = {
        .it = tokens,
        ._len = 1,
        ._scope_maps = xmalloc(sizeof(struct string_hash_map)),
        .err = err,
    };
    res._scope_maps[0] = create_string_hash_map(
        sizeof(struct identifier_type_data),
        SCOPE_MAP_INIT_CAP,
        false,
        NULL);
    return res;
}

void free_parser_state(struct parser_state* s) {
    for (size_t i = 0; i < s->_len; ++i) {
        free_string_hash_map(&s->_scope_maps[i]);
    }
    free(s->_scope_maps);
}

bool accept(struct parser_state* s, enum token_type expected) {
    if (s->it->type != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

void accept_it(struct parser_state* s) {
    assert(s->it->type != INVALID);
    ++s->it;
}

void parser_push_scope(struct parser_state* s) {
    ++s->_len;
    s->_scope_maps = xrealloc(s->_scope_maps,
                              sizeof(struct string_hash_map) * s->_len);
    s->_scope_maps[s->_len - 1] = create_string_hash_map(
        sizeof(struct identifier_type_data),
        SCOPE_MAP_INIT_CAP,
        false,
        NULL);
}

void parser_pop_scope(struct parser_state* s) {
    assert(s->_len > 1);
    --s->_len;
    free_string_hash_map(&s->_scope_maps[s->_len]);
    s->_scope_maps = xrealloc(s->_scope_maps,
                              sizeof(struct string_hash_map) * s->_len);
}

bool register_enum_constant(struct parser_state* s, const struct token* token) {
    return register_identifier(s, token, ID_TYPE_ENUM_CONSTANT);
}

bool register_typedef_name(struct parser_state* s, const struct token* token) {
    return register_identifier(s, token, ID_TYPE_TYPEDEF_NAME);
}

bool is_enum_constant(const struct parser_state* s, const struct str* spell) {
    return get_item(s, spell) == ID_TYPE_ENUM_CONSTANT;
}

bool is_typedef_name(const struct parser_state* s, const struct str* spell) {
    return get_item(s, spell) == ID_TYPE_TYPEDEF_NAME;
}

bool is_defined_in_current_scope(const struct parser_state* s,
                                   const struct str* spell) {
    const struct string_hash_map* current_map = &s->_scope_maps[s->_len - 1];
    const struct identifier_type_data* data = string_hash_map_get(current_map,
                                                                  spell);
    return data;
}

static bool register_identifier(struct parser_state* s,
                                const struct token* token,
                                enum identifier_type type) {
    assert(type != ID_TYPE_NONE);
    assert(token->type == IDENTIFIER);

    // TODO: Add a warning when an identifier from a previous scope is shadowed

    struct identifier_type_data to_insert = {
        .loc = token->loc,
        .type = type,
    };
    const struct identifier_type_data* item = string_hash_map_insert(
        &s->_scope_maps[s->_len - 1],
        &token->spelling,
        &to_insert);
    if (item != &to_insert) {
        set_parser_err(s->err, PARSER_ERR_REDEFINED_SYMBOL, token->loc);

        s->err->redefined_symbol = str_copy(&token->spelling);
        s->err->was_typedef_name = item->type == ID_TYPE_TYPEDEF_NAME;
        s->err->prev_def_file = item->loc.file_idx;
        s->err->prev_def_loc = item->loc.file_loc;
        return false;
    } else {
        return true;
    }
}

static enum identifier_type get_item(const struct parser_state* s,
                                     const struct str* spell) {
    for (size_t i = 0; i < s->_len; ++i) {
        const struct identifier_type_data* data = string_hash_map_get(
            &s->_scope_maps[i],
            spell);
        if (data != NULL) {
            return data->type;
        }
    }

    return ID_TYPE_NONE;
}
