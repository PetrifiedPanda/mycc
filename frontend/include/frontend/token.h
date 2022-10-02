#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "util/str.h"

#include "token_type.h"
#include "value.h"

struct file_loc {
    size_t line, index;
};

struct source_loc {
    size_t file_idx;
    struct file_loc file_loc;
};

struct token {
    enum token_type type;
    union {
        struct str spelling;
        struct value val;
    };
    struct source_loc loc;
};

/**
 *
 * @param type The type of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given
 *        type have only one spelling
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token(enum token_type type,
                          const struct str* spelling,
                          struct file_loc file_loc,
                          size_t file_idx);

/**
 *
 * @param type The type of the token
 * @param spelling The spelling of the token, which is to be copied, must not be
 *        NULL
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token_copy(enum token_type type,
                               const struct str* spelling,
                               struct file_loc file_loc,
                               size_t file_idx);

struct str take_spelling(struct token* t);

void free_token(struct token* t);

#endif

