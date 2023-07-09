#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

#include "util/StringMap.h"

#include "frontend/Token.h"

#include "ParserErr.h"

typedef struct {
    Token* _it;
    size_t _len, _cap;
    StringMap* _scope_maps;
    ParserErr* err;
} ParserState;

ParserState ParserState_create(Token* tokens, ParserErr* err);
void ParserState_free(ParserState* s);

bool ParserState_accept(ParserState* s, TokenKind expected);
void ParserState_accept_it(ParserState* s);

StrLit ParserState_take_curr_str_lit(ParserState* s);
StrBuf ParserState_take_curr_spell(ParserState* s);

const Token* ParserState_curr_token(const ParserState* s);
Value ParserState_curr_val(const ParserState* s);
Str ParserState_curr_spell(const ParserState* s);
TokenKind ParserState_curr_kind(const ParserState* s);
SourceLoc ParserState_curr_loc(const ParserState* s);

const Token* ParserState_next_token(const ParserState* s);
TokenKind ParserState_next_token_kind(const ParserState* s);

void ParserState_push_scope(ParserState* s);
void ParserState_pop_scope(ParserState* s);

bool ParserState_register_enum_constant(ParserState* s,
                                        const StrBuf* spell,
                                        SourceLoc loc);
bool ParserState_register_typedef(ParserState* s,
                                  const StrBuf* spell,
                                  SourceLoc loc);

bool ParserState_is_enum_constant(const ParserState* s, Str spell);
bool ParserState_is_typedef(const ParserState* s, Str spell);

typedef struct ParserIDData ParserIDData;

const ParserIDData* ParserState_get_prev_definition(const ParserState* s,
                                                    Str spell);

void ParserState_set_redefinition_err(ParserState* s,
                                      const ParserIDData* prev_def,
                                      const StrBuf* spell,
                                      SourceLoc loc);
#endif

