#ifndef MYCC_FRONTEND_PARSER_PARSER_UTIL_H
#define MYCC_FRONTEND_PARSER_PARSER_UTIL_H

#include <stddef.h>

#include "frontend/Token.h"

#include "ParserState.h"

bool is_storage_class_spec(TokenKind k);
bool is_type_qual(TokenKind k);
bool is_func_spec(TokenKind k);

/**
 *
 * @param s current state
 * @return bool whether the next token could be the start of a type name
 */
bool next_is_type_name(const ParserState* s);

bool is_type_spec(const ParserState* s);

bool is_declaration_spec(const ParserState* s);

#endif

