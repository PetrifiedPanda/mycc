#ifndef MYCC_FRONTEND_TOKEN_H
#define MYCC_FRONTEND_TOKEN_H

#include <stdbool.h>

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

_Static_assert(TOKEN_INVALID < 255, "TokenKind does not fit into 8-bit integer");

typedef struct FileLoc {
    uint32_t line, index;
} FileLoc;

typedef struct SourceLoc {
    uint32_t file_idx;
    FileLoc file_loc;
} SourceLoc;

typedef struct TokenArr {
    uint32_t len, cap;
    uint8_t* kinds;
    uint32_t* val_indices;
    SourceLoc* locs;
    StrBuf* identifiers;
    IntVal* int_consts;
    FloatVal* float_consts;
    StrLit* str_lits;
    uint32_t identifiers_len;
    uint32_t int_consts_len;
    uint32_t float_consts_len;
    uint32_t str_lits_len;
} TokenArr;

TokenArr TokenArr_create_empty(void);

void TokenArr_free(const TokenArr* arr);

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

