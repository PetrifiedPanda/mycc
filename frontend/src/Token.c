#include "frontend/Token.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

TokenArr TokenArr_create_empty(void) {
    return (TokenArr){
        .len = 0,
        .cap = 0,
        .kinds = NULL,
        .vals = NULL,
        .locs = NULL,
    };
}

void TokenArr_free(TokenArr* arr) {
    for (uint32_t i = 0; i < arr->len; ++i) {
        if (arr->kinds[i] == TOKEN_STRING_LITERAL) {
            StrLit_free(&arr->vals[i].str_lit); 
        } else if (arr->kinds[i] == TOKEN_IDENTIFIER) {
            StrBuf_free(&arr->vals[i].spelling);
        }
    }
    mycc_free(arr->kinds);
    mycc_free(arr->vals);
    mycc_free(arr->locs);
}

#ifdef _WIN32
#pragma warning(push)
// Warning comparing string literal addresses
#pragma warning(disable : 4130)
// Warning local variable initialized but not referenced
// TODO: remove this when not necessary anymore
#pragma warning(disable : 4189)
#endif

Str TokenKind_get_spelling(TokenKind kind) {
    switch (kind) {
#define TOKEN_MACRO(kind, str)                                                 \
    case kind:                                                                 \
        return str == NULL ? Str_null() : STR_LIT(str);
#include "frontend/TokenKind.inc"
#undef TOKEN_MACRO
    }
    UNREACHABLE();
}

#ifdef _WIN32
#pragma warning(pop)
#endif

Str TokenKind_str(TokenKind kind) {
    switch (kind) {
#define TOKEN_MACRO(kind, str)                                                 \
    case kind:                                                                 \
        return STR_LIT(#kind);
#include "frontend/TokenKind.inc"
#undef TOKEN_MACRO
    }
    UNREACHABLE();
}

bool TokenKind_is_rel_op(TokenKind k) {
    switch (k) {
        case TOKEN_LE:
        case TOKEN_GE:
        case TOKEN_LT:
        case TOKEN_GT:
            return true;
        default:
            return false;
    }
}

bool TokenKind_is_eq_op(TokenKind k) {
    switch (k) {
        case TOKEN_EQ:
        case TOKEN_NE:
            return true;
        default:
            return false;
    }
}

bool TokenKind_is_shift_op(TokenKind k) {
    switch (k) {
        case TOKEN_LSHIFT:
        case TOKEN_RSHIFT:
            return true;
        default:
            return false;
    }
}

bool TokenKind_is_add_op(TokenKind k) {
    switch (k) {
        case TOKEN_ADD:
        case TOKEN_SUB:
            return true;
        default:
            return false;
    }
}

bool TokenKind_is_mul_op(TokenKind k) {
    switch (k) {
        case TOKEN_ASTERISK:
        case TOKEN_DIV:
        case TOKEN_MOD:
            return true;
        default:
            return false;
    }
}

