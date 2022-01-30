#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include <stddef.h>

#include "token_type.h"
#include "token.h"
#include "parser/parser_state.h"

void expected_token_error(enum token_type expected, const struct token* got);

void expected_tokens_error(const enum token_type* expected, size_t num_expected, const struct token* got);

/**
 *
 * @param s current state
 * @return bool whether the next token could be the start of a type name
 */
bool next_is_type_name(const struct parser_state* s);

/**
 *
 * @param s The current parser_state
 * @return Whether the current token is a type specifier
 */
bool is_type_spec(const struct parser_state* s);

/**
 *
 * @param s The current parser_state
 * @return Whether the current token is a declaration_specifier
 */
bool is_declaration_spec(const struct parser_state* s);

/**
 *
 * @param s The current parser_state
 * @return Whether the current token is the start of a declaration
 */
bool is_declaration(const struct parser_state* s);

/**
 *
 * @param t A token with non-null spelling
 * @return Returns the spelling of the given token, setting it to NULL in the token
 */
char* take_spelling(struct token* t);

#endif
