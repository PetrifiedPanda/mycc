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

enum {
    SCOPE_MAP_INIT_CAP = 100
};

static bool register_identifier(ParserState* s,
                                const StrBuf* spell,
                                uint32_t idx,
                                IDKind kind);

static IDKind get_item(const ParserState* s, Str spell);

ParserState ParserState_create(TokenArr* tokens, ParserErr* err) {
    assert(tokens);

    ParserState res = {
        ._arr = *tokens,
        ._it = 0,
        ._len = 1,
        ._cap = 1,
        ._scope_maps = mycc_alloc(sizeof *res._scope_maps),
        .err = err,
    };
    res._scope_maps[0] = StringMap_create(sizeof(ParserIDData),
                                          SCOPE_MAP_INIT_CAP,
                                          false,
                                          NULL);
    *tokens = TokenArr_create_empty();
    return res;
}

void ParserState_free(ParserState* s) {
    for (uint32_t i = 0; i < s->_len; ++i) {
        StringMap_free(&s->_scope_maps[i]);
    }
    mycc_free(s->_scope_maps);
}

bool ParserState_accept(ParserState* s, TokenKind expected) {
    if (ParserState_curr_kind(s) != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->_it;
        return true;
    }
}

void ParserState_accept_it(ParserState* s) {
    assert(s->_it != s->_arr.len);
    ++s->_it;
}

Value ParserState_curr_val(const ParserState* s) {
    const TokenVal* curr = &s->_arr.vals[s->_it];
    assert(s->_arr.kinds[s->_it] == TOKEN_I_CONSTANT
           || s->_arr.kinds[s->_it] == TOKEN_F_CONSTANT);
    return curr->val;
}

Str ParserState_curr_spell(const ParserState* s) {
    const TokenVal* curr = &s->_arr.vals[s->_it];
    assert(s->_arr.kinds[s->_it] == TOKEN_IDENTIFIER);
    return StrBuf_as_str(&curr->spelling);
}

const StrBuf* ParserState_curr_spell_buf(const ParserState* s) {
    const TokenVal* curr = &s->_arr.vals[s->_it];
    assert(s->_arr.kinds[s->_it] == TOKEN_IDENTIFIER);
    return &curr->spelling;
}

TokenKind ParserState_curr_kind(const ParserState* s) {
    if (s->_it == s->_arr.len) {
        return TOKEN_INVALID;
    }
    return s->_arr.kinds[s->_it];
}

uint32_t ParserState_curr_idx(const ParserState* s) {
    return s->_it;
}

TokenKind ParserState_next_token_kind(const ParserState* s) {
    assert(s->_it < s->_arr.len - 1);
    return s->_arr.kinds[s->_it + 1];
}

Str ParserState_next_token_spell(const ParserState* s) {
    assert(s->_it < s->_arr.len - 1);
    assert(s->_arr.kinds[s->_it + 1] == TOKEN_IDENTIFIER);
    return StrBuf_as_str(&s->_arr.vals[s->_it + 1].spelling);
}

void ParserState_push_scope(ParserState* s) {
    if (s->_len == s->_cap) {
        ++s->_cap;
        s->_scope_maps = mycc_realloc(s->_scope_maps,
                                      sizeof *s->_scope_maps * s->_cap);
    }
    ++s->_len;
    s->_scope_maps[s->_len - 1] = StringMap_create(sizeof(ParserIDData),
                                                   SCOPE_MAP_INIT_CAP,
                                                   false,
                                                   NULL);
}

void ParserState_pop_scope(ParserState* s) {
    assert(s->_len > 1);
    --s->_len;
    StringMap_free(&s->_scope_maps[s->_len]);
}

bool ParserState_register_enum_constant(ParserState* s,
                                        const StrBuf* spell,
                                        uint32_t idx) {
    return register_identifier(s, spell, idx, ID_KIND_ENUM_CONSTANT);
}

bool ParserState_register_typedef(ParserState* s,
                                  const StrBuf* spell,
                                  uint32_t idx) {
    return register_identifier(s, spell, idx, ID_KIND_TYPEDEF_NAME);
}

bool ParserState_is_enum_constant(const ParserState* s, Str spell) {
    return get_item(s, spell) == ID_KIND_ENUM_CONSTANT;
}

bool ParserState_is_typedef(const ParserState* s, Str spell) {
    return get_item(s, spell) == ID_KIND_TYPEDEF_NAME;
}

const ParserIDData* ParserState_get_prev_definition(const ParserState* s,
                                                    Str spell) {
    const StringMap* current_map = &s->_scope_maps[s->_len - 1];
    return StringMap_get(current_map, spell);
}

void ParserState_set_redefinition_err(ParserState* s,
                                      const ParserIDData* prev_def,
                                      uint32_t idx) {
    ParserErr_set(s->err, PARSER_ERR_REDEFINED_SYMBOL, idx);

    s->err->was_typedef_name = prev_def->kind == ID_KIND_TYPEDEF_NAME;
    s->err->prev_def_idx = prev_def->token_idx; 
}

static bool register_identifier(ParserState* s,
                                const StrBuf* spell,
                                uint32_t idx,
                                IDKind kind) {
    assert(kind != ID_KIND_NONE);

    // TODO: Add a warning when an identifier from a previous scope is shadowed

    const ParserIDData to_insert = {
        .token_idx = idx,
        .kind = kind,
    };
    const ParserIDData* item = StringMap_insert(&s->_scope_maps[s->_len - 1],
                                                spell,
                                                &to_insert);
    if (item != &to_insert) {
        ParserState_set_redefinition_err(s, item, idx);
        return false;
    } else {
        return true;
    }
}

static IDKind get_item(const ParserState* s, Str spell) {
    for (uint32_t i = 0; i < s->_len; ++i) {
        const ParserIDData* data = StringMap_get(&s->_scope_maps[i], spell);
        if (data != NULL) {
            return data->kind;
        }
    }

    return ID_KIND_NONE;
}
