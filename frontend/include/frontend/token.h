#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "util/str.h"

#include "token_kind.h"
#include "str_lit.h"
#include "value.h"

struct file_loc {
    size_t line, index;
};

struct source_loc {
    size_t file_idx;
    struct file_loc file_loc;
};

struct token {
    enum token_kind kind;
    union {
        struct str spelling;
        struct int_value int_val;
        struct float_value float_val;
        struct str_lit str_lit;
    };
    struct source_loc loc;
};

/**
 *
 * @param kind The kind of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given
 *        kind have only one spelling
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token(enum token_kind kind,
                          const struct str* spelling,
                          struct file_loc file_loc,
                          size_t file_idx);

/**
 *
 * @param kind The kind of the token
 * @param spelling The spelling of the token, which is to be copied, must not be
 *        NULL
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
struct token create_token_copy(enum token_kind kind,
                               const struct str* spelling,
                               struct file_loc file_loc,
                               size_t file_idx);

struct str take_spelling(struct token* t);

void free_token(struct token* t);

#endif

