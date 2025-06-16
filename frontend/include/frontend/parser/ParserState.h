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

inline uint32_t ParserState_curr_id_idx(const ParserState* s);
inline TokenKind ParserState_curr_kind(const ParserState* s);

inline TokenKind ParserState_next_token_kind(const ParserState* s);
inline uint32_t ParserState_next_token_id_idx(const ParserState* s);

inline bool ParserState_accept(ParserState* s, TokenKind expected);
inline void ParserState_accept_it(ParserState* s);

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

#ifndef PARSER_STATE_INLINE
#define PARSER_STATE_INLINE inline
#endif

#include "ParserState.inc"

#undef PARSER_STATE_INLINE

#endif

