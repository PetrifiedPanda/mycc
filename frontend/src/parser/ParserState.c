#include "frontend/parser/ParserState.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

#include "frontend/parser/parser_util.h"

typedef enum {
    ID_KIND_NONE,
    ID_KIND_TYPEDEF_NAME,
    ID_KIND_ENUM_CONSTANT
} IDKind;

typedef struct ParserIDData {
    uint32_t token_idx;
    IDKind kind;
} ParserIDData;

typedef struct ParserIdentifierMap {
    uint32_t _cap;
    uint8_t* _kinds;
    uint32_t* _token_indices;
} ParserIdentifierMap;

static ParserIdentifierMap ParserIdentifierMap_create(void) {
    return (ParserIdentifierMap){0};
}

ParserState ParserState_create(TokenArr* tokens, ParserErr* err) {
    assert(tokens);

    ParserState res = {
        ._arr = *tokens,
        .it = 0,
        ._len = 1,
        ._cap = 1,
        ._scope_maps = mycc_alloc(sizeof *res._scope_maps),
        .err = err,
    };
    res._scope_maps[0] = ParserIdentifierMap_create();
    *tokens = TokenArr_create_empty();
    return res;
}

static void ParserIdentifierMap_free(const ParserIdentifierMap* map) {
    mycc_free(map->_token_indices);
    mycc_free(map->_kinds);
}

void ParserState_free(ParserState* s) {
    for (uint32_t i = 0; i < s->_cap; ++i) {
        ParserIdentifierMap_free(&s->_scope_maps[i]);
    }
    mycc_free(s->_scope_maps);
}

bool ParserState_accept(ParserState* s, TokenKind expected) {
    if (ParserState_curr_kind(s) != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

void ParserState_accept_it(ParserState* s) {
    assert(s->it != s->_arr.len);
    ++s->it;
}

uint32_t ParserState_curr_id_idx(const ParserState* s) {
    assert(s->_arr.kinds[s->it] == TOKEN_IDENTIFIER);
    return s->_arr.val_indices[s->it];
}

TokenKind ParserState_curr_kind(const ParserState* s) {
    if (s->it == s->_arr.len) {
        return TOKEN_INVALID;
    }
    return s->_arr.kinds[s->it];
}

TokenKind ParserState_next_token_kind(const ParserState* s) {
    assert(s->it < s->_arr.len - 1);
    return s->_arr.kinds[s->it + 1];
}

uint32_t ParserState_next_token_id_idx(const ParserState* s) {
    assert(s->it < s->_arr.len - 1);
    assert(s->_arr.kinds[s->it + 1] == TOKEN_IDENTIFIER);
    return s->_arr.val_indices[s->it + 1];
}

void ParserState_push_scope(ParserState* s) {
    if (s->_len == s->_cap) {
        ++s->_cap;
        s->_scope_maps = mycc_realloc(s->_scope_maps,
                                      sizeof *s->_scope_maps * s->_cap);
        s->_scope_maps[s->_len] = ParserIdentifierMap_create();
    }
    ++s->_len;
}

static void ParserIdentifierMap_clear(ParserIdentifierMap* map) {
    memset(map->_kinds, 0, sizeof *map->_kinds * map->_cap);
    // token indices don't need to be reset, because they are only accessed if
    // kind is nonzero
}

void ParserState_pop_scope(ParserState* s) {
    assert(s->_len > 1);
    --s->_len;
    ParserIdentifierMap_clear(&s->_scope_maps[s->_len]);
}

// returns whether this was already inside the map
// Does not insert if there is already an entry
static bool ParserIdentifierMap_insert(ParserIdentifierMap* map,
                                       uint32_t identifier_idx,
                                       uint32_t token_idx,
                                       IDKind kind) {
    if (identifier_idx >= map->_cap) {
        const uint32_t new_cap = identifier_idx + 1;
        map->_kinds = mycc_realloc(map->_kinds, sizeof *map->_kinds * new_cap);
        map->_token_indices = mycc_realloc(map->_token_indices, sizeof *map->_token_indices * new_cap);
        memset(map->_kinds + map->_cap, 0, sizeof *map->_kinds * (new_cap - map->_cap));
        map->_cap = new_cap;
    } else if (map->_kinds[identifier_idx] != ID_KIND_NONE) {
        return false;
    }
    map->_kinds[identifier_idx] = kind;
    map->_token_indices[identifier_idx] = token_idx;
    return true;
}

static bool register_identifier(ParserState* s,
                                uint32_t identifier_idx,
                                uint32_t token_idx,
                                IDKind kind) {
    assert(kind != ID_KIND_NONE);
    // TODO: shadowing warning
    if (ParserIdentifierMap_insert(&s->_scope_maps[s->_len - 1], identifier_idx,
                                   token_idx, kind)) {
        return true;
    } else {
        ParserState_set_redefinition_err(s, identifier_idx, token_idx);
        return false;
    }
}

bool ParserState_register_enum_constant(ParserState* s,
                                        uint32_t identifier_idx,
                                        uint32_t token_idx) {
    return register_identifier(s, identifier_idx, token_idx, ID_KIND_ENUM_CONSTANT);
}

bool ParserState_register_typedef(ParserState* s,
                                  uint32_t identifier_idx,
                                  uint32_t token_idx) {
    return register_identifier(s, identifier_idx, token_idx, ID_KIND_TYPEDEF_NAME);
}

static IDKind ParserIdentifierMap_get_kind(const ParserIdentifierMap* map, uint32_t identifier_idx) {
    if (identifier_idx >= map->_cap) {
        return ID_KIND_NONE;
    }
    return map->_kinds[identifier_idx];
}

static IDKind get_item(const ParserState* s, uint32_t identifier_idx) {
    for (uint32_t i = 0; i < s->_len; ++i) {
        const IDKind kind = ParserIdentifierMap_get_kind(&s->_scope_maps[i], identifier_idx);
        if (kind != ID_KIND_NONE) {
            return kind;
        }
    }

    return ID_KIND_NONE;
}

bool ParserState_is_defined_in_current_scope(const ParserState* s, uint32_t identifier_idx) {
    const IDKind kind = ParserIdentifierMap_get_kind(&s->_scope_maps[s->_len - 1],
                                                     identifier_idx);
    return kind != ID_KIND_NONE;
}

bool ParserState_is_enum_constant(const ParserState* s, uint32_t identifier_idx) {
    return get_item(s, identifier_idx) == ID_KIND_ENUM_CONSTANT;
}

bool ParserState_is_typedef(const ParserState* s, uint32_t identifier_idx) {
    return get_item(s, identifier_idx) == ID_KIND_TYPEDEF_NAME;
}

static ParserIDData ParserIdentifierMap_get(const ParserIdentifierMap* map, uint32_t identifier_idx) {
    if (identifier_idx >= map->_cap) {
        return (ParserIDData){0};
    }
    return (ParserIDData){
        .kind = map->_kinds[identifier_idx],
        .token_idx = map->_token_indices[identifier_idx],
    };
}

void ParserState_set_redefinition_err(ParserState* s,
                                      uint32_t identifier_idx,
                                      uint32_t redef_token_idx) {

    ParserErr_set(s->err, PARSER_ERR_REDEFINED_SYMBOL, redef_token_idx);

    ParserIDData prev_def = ParserIdentifierMap_get(&s->_scope_maps[s->_len - 1], identifier_idx);
    assert(prev_def.kind != ID_KIND_NONE);

    s->err->was_typedef_name = prev_def.kind == ID_KIND_TYPEDEF_NAME;
    s->err->prev_def_idx = prev_def.token_idx; 
}

