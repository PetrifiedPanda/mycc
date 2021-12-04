#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "token_type.h"

typedef struct {
    size_t line, index;
} SourceLocation;

typedef struct {
    TokenType type;
    char* spelling;
    char* file;
    SourceLocation source_loc;
} Token;

/**
 * @brief Initialize the token given in the pointer
 * 
 * @param t The token to initialize, must not be NULL
 * @param type The type of the token
 * @param spelling The spelling of the token, or NULL if Tokens of the given type have only one spelling
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
void init_token(Token* t, TokenType type, char* spelling, SourceLocation loc, const char* filename);

/**
 * @brief Initialize the token in the given pointer, copying the spelling
 * 
 * @param t The token to initialize, must not be NULL
 * @param type The type of the token
 * @param spelling The spelling of the token, which is to be copied, must not be NULL
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
void init_token_copy(Token* t, TokenType type, const char* spelling, SourceLocation loc, const char* filename);

void free_token(Token* t);

#endif

