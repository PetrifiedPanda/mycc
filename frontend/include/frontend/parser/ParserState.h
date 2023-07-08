#ifndef PARSER_STATE_H
#define PARSER_STATE_H

#include <stdbool.h>

#include "util/StringMap.h"

#include "frontend/Token.h"

#include "ParserErr.h"

typedef struct {
    Token* it;
    size_t _len, _cap;
    StringMap* _scope_maps;
    ParserErr* err;
} ParserState;

ParserState ParserState_create(Token* tokens, ParserErr* err);
void ParserState_free(ParserState* s);

bool parser_accept(ParserState* s, TokenKind expected);
void parser_accept_it(ParserState* s);

StrLit ParserState_take_curr_str_lit(ParserState* s);
StrBuf ParserState_take_curr_spell(ParserState* s);

const StrBuf* ParserState_curr_spell_buf(const ParserState* s);
Value ParserState_curr_val(const ParserState* s);
Str ParserState_curr_spell(const ParserState* s);
TokenKind ParserState_curr_kind(const ParserState* s);
SourceLoc ParserState_curr_loc(const ParserState* s);

const Token* ParserState_next_token(const ParserState* s);
TokenKind ParserState_next_token_kind(const ParserState* s);

void parser_push_scope(ParserState* s);
void parser_pop_scope(ParserState* s);

bool parser_register_enum_constant(ParserState* s,
                                   const StrBuf* spell,
                                   SourceLoc loc);
bool parser_register_typedef_name(ParserState* s,
                                  const StrBuf* spell,
                                  SourceLoc loc);

bool parser_is_enum_constant(const ParserState* s, Str spell);
bool parser_is_typedef_name(const ParserState* s, Str spell);

typedef struct ParserIdentifierData ParserIdentifierData;

const ParserIdentifierData* parser_get_prev_definition(const ParserState* s,
                                                       Str spell);

void parser_set_redefinition_err(ParserState* s,
                                 const ParserIdentifierData* prev_def,
                                 const StrBuf* spell,
                                 SourceLoc loc);
#endif

