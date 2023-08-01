#ifndef MYCC_FRONTEND_TOKEN_H
#define MYCC_FRONTEND_TOKEN_H

#include <stdbool.h>
#include <stddef.h>

#include "util/StrBuf.h"

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
    uint32_t line, index;
} FileLoc;

typedef struct {
    uint32_t file_idx;
    FileLoc file_loc;
} SourceLoc;

typedef struct {
    TokenKind kind;
    union {
        StrBuf spelling;
        Value val;
        StrLit str_lit;
    };
    SourceLoc loc;
} Token;

typedef struct {
    uint32_t len, cap;
    Token* tokens;
} TokenArr;



/**
 *
 * @param kind The kind of the token
 * @param spelling The spelling of the token, or NULL if tokens of the given
 *        kind have only one spelling
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
Token Token_create(TokenKind kind,
                   const StrBuf* spelling,
                   FileLoc file_loc,
                   uint32_t file_idx);

/**
 *
 * @param kind The kind of the token
 * @param spelling The spelling of the token, which is to be copied, must not be
 *        NULL
 * @param file_loc The location of the token in the file
 * @param filename The file this token is in (This is copied into the token)
 */
Token Token_create_copy(TokenKind kind,
                        const StrBuf* spelling,
                        FileLoc file_loc,
                        uint32_t file_idx);

StrBuf Token_take_spelling(Token* t);

StrLit Token_take_str_lit(Token* t);

void Token_free(Token* t);

void TokenArr_free(TokenArr* arr);

/**
 * @brief Gets a spelling for the given token_kind
 *
 * @param kind Type to get the spelling for
 * @return The spelling of the given token kind, if it is
 * unambiguous, otherwise NULL
 */
Str TokenKind_get_spelling(TokenKind kind);

/**
 * @brief Gets a string to identify the token_kind
 *
 * @return A string that is identical to the spelling of the enum
 * value
 */
Str TokenKind_str(TokenKind kind);

bool TokenKind_is_rel_op(TokenKind k);
bool TokenKind_is_eq_op(TokenKind k);
bool TokenKind_is_shift_op(TokenKind k);
bool TokenKind_is_add_op(TokenKind k);
bool TokenKind_is_mul_op(TokenKind k);

#endif

