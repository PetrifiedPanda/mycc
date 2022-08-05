#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include <stddef.h>

#include "frontend/token_type.h"
#include "frontend/token.h"
#include "parser_state.h"

void expected_token_error(struct parser_state* s, enum token_type expected);

void expected_tokens_error(struct parser_state* s,
                           const enum token_type* expected,
                           size_t num_expected);

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
 * @return Whether the current token is the start of a declaration
 */
bool is_declaration(const struct parser_state* s);

/**
 *
 * @param t A token with non-null spelling
 * @return Returns the spelling of the given token, setting it to NULL in the
 * token
 */
char* take_spelling(struct token* t);

#endif
