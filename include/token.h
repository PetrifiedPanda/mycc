#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "token_type.h"

struct source_location {
    size_t line, index;
};

struct token {
    enum token_type type;
    char* spelling;
    char* file;
    struct source_location source_loc;
};

/**
 * @brief Initialize the token given in the pointer
 * 
 * @param t The token to initialize, must not be NULL
 * @param type The type of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given type have only one spelling
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
void init_token(struct token* t, enum token_type type, char* spelling, struct source_location loc, const char* filename);

/**
 * @brief Initialize the token in the given pointer, copying the spelling
 * 
 * @param t The token to initialize, must not be NULL
 * @param type The type of the token
 * @param spelling The spelling of the token, which is to be copied, must not be NULL
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
void init_token_copy(struct token* t, enum token_type type, const char* spelling, struct source_location loc, const char* filename);

void free_token(struct token* t);

#endif

