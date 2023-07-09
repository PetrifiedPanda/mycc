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
    SourceLoc loc;
    IDKind kind;
} ParserIDData;

enum {
    SCOPE_MAP_INIT_CAP = 100
};

static bool register_identifier(ParserState* s,
                                const StrBuf* spell,
                                SourceLoc loc,
                                IDKind kind);

static IDKind get_item(const ParserState* s, Str spell);

ParserState ParserState_create(Token* tokens, ParserErr* err) {
    assert(tokens);

    ParserState res = {
        ._it = tokens,
        ._len = 1,
        ._cap = 1,
        ._scope_maps = mycc_alloc(sizeof *res._scope_maps),
        .err = err,
    };
    res._scope_maps[0] = StringMap_create(sizeof(ParserIDData),
                                          SCOPE_MAP_INIT_CAP,
                                          false,
                                          NULL);
    return res;
}

void ParserState_free(ParserState* s) {
    for (size_t i = 0; i < s->_len; ++i) {
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
    assert(s->_it->kind != TOKEN_INVALID);
    ++s->_it;
}

StrLit ParserState_take_curr_str_lit(ParserState* s) {
    assert(s->_it->kind == TOKEN_STRING_LITERAL);
    return Token_take_str_lit(s->_it);
}

StrBuf ParserState_take_curr_spell(ParserState* s) {
    assert(s->_it->kind == TOKEN_IDENTIFIER);
    return Token_take_spelling(s->_it);
}

const Token* ParserState_curr_token(const ParserState* s) {
    return s->_it;
}

Value ParserState_curr_val(const ParserState* s) {
    assert(s->_it->kind == TOKEN_I_CONSTANT
           || s->_it->kind == TOKEN_F_CONSTANT);
    return s->_it->val;
}

Str ParserState_curr_spell(const ParserState* s) {
    assert(s->_it->kind == TOKEN_IDENTIFIER);
    return StrBuf_as_str(&s->_it->spelling);
}

TokenKind ParserState_curr_kind(const ParserState* s) {
    return s->_it->kind;
}

SourceLoc ParserState_curr_loc(const ParserState* s) {
    return s->_it->loc;
}

const Token* ParserState_next_token(const ParserState* s) {
    assert(s->_it->kind != TOKEN_INVALID);
    return &s->_it[1];
}

TokenKind ParserState_next_token_kind(const ParserState* s) {
    assert(s->_it->kind != TOKEN_INVALID);
    return s->_it[1].kind;
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
                                        SourceLoc loc) {
    return register_identifier(s, spell, loc, ID_KIND_ENUM_CONSTANT);
}

bool ParserState_register_typedef(ParserState* s,
                                  const StrBuf* spell,
                                  SourceLoc loc) {
    return register_identifier(s, spell, loc, ID_KIND_TYPEDEF_NAME);
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
                                      const StrBuf* spell,
                                      SourceLoc loc) {
    ParserErr_set(s->err, PARSER_ERR_REDEFINED_SYMBOL, loc);

    s->err->redefined_symbol = *spell;
    s->err->was_typedef_name = prev_def->kind == ID_KIND_TYPEDEF_NAME;
    s->err->prev_def_file = prev_def->loc.file_idx;
    s->err->prev_def_loc = prev_def->loc.file_loc;
}

static bool register_identifier(ParserState* s,
                                const StrBuf* spell,
                                SourceLoc loc,
                                IDKind kind) {
    assert(kind != ID_KIND_NONE);

    // TODO: Add a warning when an identifier from a previous scope is shadowed

    const ParserIDData to_insert = {
        .loc = loc,
        .kind = kind,
    };
    const ParserIDData* item = StringMap_insert(&s->_scope_maps[s->_len - 1],
                                                spell,
                                                &to_insert);
    if (item != &to_insert) {
        const StrBuf copy = StrBuf_copy(spell);
        ParserState_set_redefinition_err(s, item, &copy, loc);
        return false;
    } else {
        return true;
    }
}

static IDKind get_item(const ParserState* s, Str spell) {
    for (size_t i = 0; i < s->_len; ++i) {
        const ParserIDData* data = StringMap_get(&s->_scope_maps[i], spell);
        if (data != NULL) {
            return data->kind;
        }
    }

    return ID_KIND_NONE;
}
