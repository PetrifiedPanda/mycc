#ifndef MYCC_FRONTEND_PARSER_PARSER_UTIL_H
#define MYCC_FRONTEND_PARSER_PARSER_UTIL_H

#include <stddef.h>

#include "frontend/Token.h"

#include "ParserState.h"

void expected_token_error(ParserState* s, TokenKind expected);

void expected_tokens_error(ParserState* s,
                           const TokenKind* expected,
                           uint32_t num_expected);

bool is_storage_class_spec(TokenKind k);
bool is_type_qual(TokenKind k);
bool is_func_spec(TokenKind k);

/**
 *
 * @param s current state
 * @return bool whether the next token could be the start of a type name
 */
bool next_is_type_name(const ParserState* s);

/**
* @param s current state
* @return bool whether the next token is a type specifier
*/
bool next_is_type_spec(const ParserState* s);

/**
 *
 * @param s current state
 * @param token token to check
 * @return whether token is a type specifier
 */
bool is_type_spec_token(const ParserState* s,
                        TokenKind kind,
                        uint32_t identifier_idx);

bool is_type_spec(const ParserState* s);

bool is_declaration_spec(const ParserState* s);

/**
 *
 * @param s The current parser_state
 * @return Whether the current token is the start of a declaration
 */
bool is_declaration(const ParserState* s);

#endif

