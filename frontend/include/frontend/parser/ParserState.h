#ifndef MYCC_FRONTEND_PARSER_PARSER_STATE_H
#define MYCC_FRONTEND_PARSER_PARSER_STATE_H

#include <stdbool.h>

#include "util/StringMap.h"

#include "frontend/Token.h"

#include "ParserErr.h"

typedef struct ParserState {
    TokenArr _arr;
    uint32_t it;
    uint32_t _len, _cap;
    StringMap* _scope_maps;
    ParserErr* err;
} ParserState;

ParserState ParserState_create(TokenArr* tokens, ParserErr* err);
void ParserState_free(ParserState* s);

bool ParserState_accept(ParserState* s, TokenKind expected);
void ParserState_accept_it(ParserState* s);

IntVal ParserState_curr_int_val(const ParserState* s);
FloatVal ParserState_curr_float_val(const ParserState* s);

Str ParserState_curr_spell(const ParserState* s);
const StrBuf* ParserState_curr_spell_buf(const ParserState* s);
TokenKind ParserState_curr_kind(const ParserState* s);

TokenKind ParserState_next_token_kind(const ParserState* s);
Str ParserState_next_token_spell(const ParserState* s);

void ParserState_push_scope(ParserState* s);
void ParserState_pop_scope(ParserState* s);

bool ParserState_register_enum_constant(ParserState* s,
                                        const StrBuf* spell,
                                        uint32_t idx);
bool ParserState_register_typedef(ParserState* s,
                                  const StrBuf* spell,
                                  uint32_t idx);

bool ParserState_is_enum_constant(const ParserState* s, Str spell);
bool ParserState_is_typedef(const ParserState* s, Str spell);

typedef struct ParserIDData ParserIDData;

const ParserIDData* ParserState_get_prev_definition(const ParserState* s,
                                                    Str spell);

void ParserState_set_redefinition_err(ParserState* s,
                                      const ParserIDData* prev_def,
                                      uint32_t idx);
#endif

