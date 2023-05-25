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

ParserState create_parser_state(Token* tokens, ParserErr* err);
void free_parser_state(ParserState* s);

bool parser_accept(ParserState* s, TokenKind expected);
void parser_accept_it(ParserState* s);

void parser_push_scope(ParserState* s);
void parser_pop_scope(ParserState* s);

bool parser_register_enum_constant(ParserState* s, const Token* token);
bool parser_register_typedef_name(ParserState* s, const Token* token);

bool parser_is_enum_constant(const ParserState* s, const Str* spell);
bool parser_is_typedef_name(const ParserState* s, const Str* spell);

typedef struct ParserIdentifierData ParserIdentifierData;

const ParserIdentifierData* parser_get_prev_definition(const ParserState* s, const Str* spell);

void parser_set_redefinition_err(ParserState* s,
                                 const ParserIdentifierData* prev_def,
                                 const Token* redef_tok);
#endif

