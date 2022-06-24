#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "token_type.h"

struct file_loc {
    size_t line, index;
};

struct source_loc {
    char* file;
    struct file_loc file_loc;
};

struct token {
    enum token_type type;
    char* spelling;
    struct source_loc loc;
};

/**
 *
 * @param type The type of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given
 * type have only one spelling
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token(enum token_type type,
                          char* spelling,
                          struct file_loc file_loc,
                          const char* filename);

/**
 *
 * @param type The type of the token
 * @param spelling The spelling of the token, which is to be copied, must not be
 * NULL
 * @param loc The source location of the token
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token_copy(enum token_type type,
                               const char* spelling,
                               struct file_loc file_loc,
                               const char* filename);

void free_source_loc(struct source_loc* loc);

void free_token(struct token* t);

#endif

