#include "ParserState.h"

PARSER_STATE_INLINE uint32_t ParserState_curr_id_idx(const ParserState* s) {
    assert(s->_arr.kinds[s->it] == TOKEN_IDENTIFIER);
    return s->_arr.val_indices[s->it];
}

PARSER_STATE_INLINE TokenKind ParserState_curr_kind(const ParserState* s) {
    if (s->it == s->_arr.len) {
        return TOKEN_INVALID;
    }
    return s->_arr.kinds[s->it];
}

PARSER_STATE_INLINE TokenKind ParserState_next_token_kind(const ParserState* s) {
    assert(s->it < s->_arr.len - 1);
    return s->_arr.kinds[s->it + 1];
}

PARSER_STATE_INLINE uint32_t ParserState_next_token_id_idx(const ParserState* s) {
    assert(s->it < s->_arr.len - 1);
    assert(s->_arr.kinds[s->it + 1] == TOKEN_IDENTIFIER);
    return s->_arr.val_indices[s->it + 1];
}

PARSER_STATE_INLINE bool ParserState_accept(ParserState* s, TokenKind expected) {
    if (ParserState_curr_kind(s) != expected) {
        expected_token_error(s, expected);
        return false;
    } else {
        ++s->it;
        return true;
    }
}

PARSER_STATE_INLINE void ParserState_accept_it(ParserState* s) {
    assert(s->it != s->_arr.len);
    ++s->it;
}
