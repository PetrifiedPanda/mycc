#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "util/Str.h"

#include "StrLit.h"
#include "Value.h"

typedef enum {
#define TOKEN_MACRO(name, str) name,
#define META_TOKEN_MACRO(name, synonym) name = synonym,
#include "TokenKind.inc"
#undef TOKEN_MACRO
#undef META_TOKEN_MACRO
} TokenKind;

typedef struct {
    size_t line, index;
} FileLoc;

typedef struct {
    size_t file_idx;
    FileLoc file_loc;
} SourceLoc;

typedef struct {
    TokenKind kind;
    union {
        Str spelling;
        IntValue int_val;
        FloatValue float_val;
        StrLit str_lit;
    };
    SourceLoc loc;
} Token;

/**
 *
 * @param kind The kind of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given
 *        kind have only one spelling
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
Token Token_create(TokenKind kind,
                   const Str* spelling,
                   FileLoc file_loc,
                   size_t file_idx);

/**
 *
 * @param kind The kind of the token
 * @param spelling The spelling of the token, which is to be copied, must not be
 *        NULL
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
Token Token_create_copy(TokenKind kind,
                        const Str* spelling,
                        FileLoc file_loc,
                        size_t file_idx);

Str Token_take_spelling(Token* t);

StrLit Token_take_str_lit(Token* t);

void Token_free(Token* t);

/**
 * @brief Gets a spelling for the given token_kind
 *
 * @param kind Type to get the spelling for
 * @return const char* The spelling of the given token kind, if it is
 * unambiguous, otherwise NULL
 */
const char* TokenKind_get_spelling(TokenKind kind);

/**
 * @brief Gets a string to identify the token_kind
 *
 * @return const char* A string that is identical to the spelling of the enum
 * value
 */
const char* TokenKind_str(TokenKind kind);

#endif

