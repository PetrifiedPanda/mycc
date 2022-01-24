#include "parser/parser_state.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "error.h"
#include "util.h"

#include "parser/parser_util.h"

enum identifier_type {
    ID_TYPE_NONE,
    ID_TYPE_TYPEDEF_NAME,
    ID_TYPE_ENUM_CONSTANT
};

/**
 * The pointers stored in this need to be valid at least as long
 * as the parser_state
 */
struct identifier_type_pair {
    const char* spelling;
    struct source_location source_loc;
    const char* file;
    enum identifier_type type;
};

struct identifier_type_map {
    struct identifier_type_pair* pairs;
    size_t len;
    size_t cap;
};

// Hash function taken from K&R version 2 (page 144)
static size_t hash_string(const char* str) {
    size_t hash = 0;

    const char* it = str;
    while (*it != '\0') {
        hash = *it + 31 * hash;
        ++it;
    }

    return hash;
}

static bool insert_identifier(struct identifier_type_map* map, const struct identifier_type_pair* item);

static void resize_map(struct identifier_type_map* map) {
    const size_t prev_cap = map->cap;
    const size_t prev_len = map->len;
    struct identifier_type_pair* old_pairs = map->pairs;

    map->len = 0;
    map->cap += map->cap / 2 + 1;
    map->pairs = xcalloc(map->cap, sizeof(struct identifier_type_pair));

    for (size_t i = 0; i < prev_cap; ++i) {
        if (old_pairs[i].spelling != NULL) {
            bool success = insert_identifier(map, &old_pairs[i]);
            UNUSED(success);
            assert(success);
        }
    }

    UNUSED(prev_len);
    assert(map->len == prev_len);
    free(old_pairs);
}

static bool insert_identifier(struct identifier_type_map* map, const struct identifier_type_pair* item) {
    assert(item->spelling);
    assert(item->file);
    assert(item->type != ID_TYPE_NONE);

    if (map->len == map->cap) {
        resize_map(map);
    }

    const size_t hash = hash_string(item->spelling);
    size_t i = hash != 0 ? hash % map->cap : hash;
    while (map->pairs[i].spelling != NULL && strcmp(map->pairs[i].spelling, item->spelling) != 0) {
        ++i;
        if (i == map->cap) {
            i = 0;
        }
    }

    if (map->pairs[i].spelling != NULL) {
        const char* type_string = map->pairs->type == ID_TYPE_ENUM_CONSTANT ? "enum constant" : "typedef name";
        struct source_location loc = map->pairs[i].source_loc;
        set_error_file(ERR_PARSER, item->file, item->source_loc, "Redefined symbol %s that is already defined as %s in %s(%zu,%zu)", map->pairs[i].spelling, type_string, loc.line, loc.index);
        return false;
    }

    map->pairs[i] = *item;
    ++map->len;
    return true;
}

static enum identifier_type get_item(const struct identifier_type_map* map, const char* spelling) {
    assert(spelling);

    const size_t hash = hash_string(spelling);
    size_t i = hash != 0 ? hash % map->cap : hash;
    while (map->pairs[i].spelling != NULL && strcmp(map->pairs[i].spelling, spelling) != 0) {
        ++i;
        if (i == map->cap) {
            i = 0;
        }
    }

    if (map->pairs[i].spelling == NULL) {
        return ID_TYPE_NONE;
    }

    return map->pairs[i].type;
}

static struct identifier_type_map create_id_type_map() {
    enum {INIT_LEN = 100};
    return (struct identifier_type_map) {
        .pairs = xcalloc(INIT_LEN, sizeof(struct identifier_type_pair)),
        .len = 0,
        .cap = INIT_LEN
    };
}

struct parser_state create_parser_state(struct token* tokens) {
    assert(tokens);

    struct parser_state res = {
        .it = tokens,
        .len = 1,
        .scope_maps = xmalloc(sizeof(struct identifier_type_map))
    };
    res.scope_maps[0] = create_id_type_map();
    return res;
}

void free_parser_state(struct parser_state* s) {
    for (size_t i = 0; i < s->len; ++i) {
        free(s->scope_maps[i].pairs);
    }
    free(s->scope_maps);
}

bool accept(struct parser_state* s, enum token_type expected) {
    if (s->it->type != expected) {
        expected_token_error(expected, s->it);
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
    ++s->len;
    s->scope_maps = xrealloc(s->scope_maps, sizeof(struct identifier_type_map) * s->len);
    s->scope_maps[s->len - 1] = create_id_type_map();
}

void parser_pop_scope(struct parser_state* s) {
    assert(s->len > 1);
    --s->len;
    free(s->scope_maps[s->len].pairs);
    s->scope_maps = xrealloc(s->scope_maps, sizeof(struct identifier_type_map) * s->len);
}

static bool register_identifier(struct parser_state* s, const struct token* token, enum identifier_type type) {
    assert(type != ID_TYPE_NONE);
    assert(token->type == IDENTIFIER);

    struct identifier_type_pair to_insert = {
            .spelling = token->spelling,
            .source_loc = token->source_loc,
            .file = token->file,
            .type = type
    };
    return insert_identifier(&s->scope_maps[s->len - 1], &to_insert);
}

bool register_enum_constant(struct parser_state* s, const struct token* token) {
    return register_identifier(s, token, ID_TYPE_ENUM_CONSTANT);
}

bool register_typedef_name(struct parser_state* s, const struct token* token) {
    return register_identifier(s, token, ID_TYPE_TYPEDEF_NAME);
}

static enum identifier_type get_item_type(const struct parser_state* s, const char* spell) {
    for (size_t i = 0; i < s->len; ++i) {
        enum identifier_type type = get_item(&s->scope_maps[i], spell);
        if (type != ID_TYPE_NONE) {
            return type;
        }
    }

    return ID_TYPE_NONE;
}

bool is_enum_constant(const struct parser_state* s, const char* spell) {
    return get_item_type(s, spell) == ID_TYPE_ENUM_CONSTANT;
}

bool is_typedef_name(const struct parser_state* s, const char* spell) {
    return get_item_type(s, spell) == ID_TYPE_TYPEDEF_NAME;
}
