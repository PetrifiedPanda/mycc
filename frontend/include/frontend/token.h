#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "util/str.h"

#include "str_lit.h"
#include "value.h"

enum token_kind {
#define TOKEN_MACRO(name, str) name,
#define META_TOKEN_MACRO(name, synonym) name = synonym,
#include "token_kind.inc"
#undef TOKEN_MACRO
#undef META_TOKEN_MACRO
};

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

struct str token_take_spelling(struct token* t);

struct str_lit token_take_str_lit(struct token* t);

void free_token(struct token* t);

/**
 * @brief Gets a spelling for the given token_kind
 *
 * @param kind Type to get the spelling for
 * @return const char* The spelling of the given token kind, if it is
 * unambiguous, otherwise NULL
 */
const char* get_token_kind_spelling(enum token_kind kind);

/**
 * @brief Gets a string to identify the token_kind
 * 
 * @return const char* A string that is identical to the spelling of the enum
 * value
 */
const char* get_token_kind_str(enum token_kind kind);

#endif

