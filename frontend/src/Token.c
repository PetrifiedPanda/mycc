#include "frontend/Token.h"

#include "util/mem.h"
#include "util/macro_util.h"

TokenArr TokenArr_create_empty(void) {
    return (TokenArr){
        .len = 0,
        .cap = 0,
        .kinds = NULL,
        .locs = NULL,
    };
}

void TokenArr_free(const TokenArr* arr) {
    mycc_free(arr->kinds);
    mycc_free(arr->val_indices);
    mycc_free(arr->locs);
    for (uint32_t i = 0; i < arr->identifiers_len; ++i) {
        StrBuf_free(&arr->identifiers[i]);
    }
    mycc_free(arr->identifiers);
    mycc_free(arr->int_consts);
    mycc_free(arr->float_consts);
    for (uint32_t i = 0; i < arr->str_lits_len; ++i) {
        StrLit_free(&arr->str_lits[i]);
    }
    mycc_free(arr->str_lits);
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

