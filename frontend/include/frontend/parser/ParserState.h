#ifndef MYCC_FRONTEND_PARSER_PARSER_STATE_H
#define MYCC_FRONTEND_PARSER_PARSER_STATE_H

#include <stdbool.h>

#include "frontend/Token.h"

#include "ParserErr.h"

typedef struct ParserIdentifierMap ParserIdentifierMap;

typedef struct ParserState {
    TokenArr _arr;
    uint32_t it;
    uint32_t _len, _cap;
    ParserIdentifierMap* _scope_maps;
    ParserErr* err;
} ParserState;

ParserState ParserState_create(TokenArr* tokens, ParserErr* err);
void ParserState_free(ParserState* s);

void expected_token_error(ParserState* s, TokenKind expected);

void expected_tokens_error(ParserState* s,
                           const TokenKind* expected,
                           uint32_t num_expected);

inline uint32_t ParserState_curr_id_idx(const ParserState* s) {
    assert(s->_arr.kinds[s->it] == TOKEN_IDENTIFIER);
    return s->_arr.val_indices[s->it];
}

inline TokenKind ParserState_curr_kind(const ParserState* s) {
    if (s->it == s->_arr.len) {
        return TOKEN_INVALID;
    }
    return s->_arr.kinds[s->it];
}

inline TokenKind ParserState_next_token_kind(const ParserState* s) {
    assert(s->it < s->_arr.len - 1);
    return s->_arr.kinds[s->it + 1];
}

inline uint32_t ParserState_next_token_id_idx(const ParserState* s) {
    assert(s->it < s->_arr.len - 1);
    assert(s->_arr.kinds[s->it + 1] == TOKEN_IDENTIFIER);
    return s->_arr.val_indices[s->it + 1];
}

inline bool ParserState_accept(ParserState* s, TokenKind expected) {
    if (ParserState_curr_kind(s) != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

inline void ParserState_accept_it(ParserState* s) {
    assert(s->it != s->_arr.len);
    ++s->it;
}

void ParserState_push_scope(ParserState* s);
void ParserState_pop_scope(ParserState* s);

bool ParserState_register_enum_constant(ParserState* s,
                                        uint32_t identifier_idx,
                                        uint32_t token_idx);
bool ParserState_register_typedef(ParserState* s,
                                  uint32_t identifier_idx,
                                  uint32_t token_idx);

bool ParserState_is_enum_constant(const ParserState* s, uint32_t identifier_idx);
bool ParserState_is_typedef(const ParserState* s, uint32_t identifier_idx);

void ParserState_set_redefinition_err(ParserState* s,
                                      uint32_t identifier_idx,
                                      uint32_t redef_token_idx);

#endif

