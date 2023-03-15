#include "frontend/parser/parser_state.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

enum identifier_kind {
    ID_TYPE_NONE,
    ID_TYPE_TYPEDEF_NAME,
    ID_TYPE_ENUM_CONSTANT
};

struct parser_identifier_data {
    struct source_loc loc;
    enum identifier_kind kind;
};

enum {
    SCOPE_MAP_INIT_CAP = 100
};

static bool register_identifier(struct parser_state* s,
                                const struct token* token,
                                enum identifier_kind kind);

static enum identifier_kind get_item(const struct parser_state* s,
                                     const struct str* spell);

struct parser_state create_parser_state(struct token* tokens,
                                        struct parser_err* err) {
    assert(tokens);

    struct parser_state res = {
        .it = tokens,
        ._len = 1,
        ._scope_maps = mycc_alloc(sizeof *res._scope_maps),
        .err = err,
    };
    res._scope_maps[0] = create_string_hash_map(
        sizeof(struct parser_identifier_data),
        SCOPE_MAP_INIT_CAP,
        false,
        NULL);
    return res;
}

void free_parser_state(struct parser_state* s) {
    for (size_t i = 0; i < s->_len; ++i) {
        free_string_hash_map(&s->_scope_maps[i]);
    }
    mycc_free(s->_scope_maps);
}

bool accept(struct parser_state* s, enum token_kind expected) {
    if (s->it->kind != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

void accept_it(struct parser_state* s) {
    assert(s->it->kind != INVALID);
    ++s->it;
}

void parser_push_scope(struct parser_state* s) {
    ++s->_len;
    s->_scope_maps = mycc_realloc(s->_scope_maps,
                                  sizeof *s->_scope_maps * s->_len);
    s->_scope_maps[s->_len - 1] = create_string_hash_map(
        sizeof(struct parser_identifier_data),
        SCOPE_MAP_INIT_CAP,
        false,
        NULL);
}

void parser_pop_scope(struct parser_state* s) {
    assert(s->_len > 1);
    --s->_len;
    free_string_hash_map(&s->_scope_maps[s->_len]);
    s->_scope_maps = mycc_realloc(s->_scope_maps,
                                  sizeof *s->_scope_maps * s->_len);
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

const struct parser_identifier_data* parser_get_prev_definition(
    const struct parser_state* s,
    const struct str* spell) {
    const struct string_hash_map* current_map = &s->_scope_maps[s->_len - 1];
    return string_hash_map_get(current_map, spell);
}

void parser_set_redefinition_err(struct parser_state* s,
                                 const struct parser_identifier_data* prev_def,
                                 const struct token* redef_tok) {
    set_parser_err(s->err, PARSER_ERR_REDEFINED_SYMBOL, redef_tok->loc);

    s->err->redefined_symbol = str_copy(&redef_tok->spelling);
    s->err->was_typedef_name = prev_def->kind == ID_TYPE_TYPEDEF_NAME;
    s->err->prev_def_file = prev_def->loc.file_idx;
    s->err->prev_def_loc = prev_def->loc.file_loc;
}

static bool register_identifier(struct parser_state* s,
                                const struct token* token,
                                enum identifier_kind kind) {
    assert(kind != ID_TYPE_NONE);
    assert(token->kind == IDENTIFIER);

    // TODO: Add a warning when an identifier from a previous scope is shadowed

    struct parser_identifier_data to_insert = {
        .loc = token->loc,
        .kind = kind,
    };
    const struct parser_identifier_data* item = string_hash_map_insert(
        &s->_scope_maps[s->_len - 1],
        &token->spelling,
        &to_insert);
    if (item != &to_insert) {
        parser_set_redefinition_err(s, item, token);
        return false;
    } else {
        return true;
    }
}

static enum identifier_kind get_item(const struct parser_state* s,
                                     const struct str* spell) {
    for (size_t i = 0; i < s->_len; ++i) {
        const struct parser_identifier_data* data = string_hash_map_get(
            &s->_scope_maps[i],
            spell);
        if (data != NULL) {
            return data->kind;
        }
    }

    return ID_TYPE_NONE;
}
