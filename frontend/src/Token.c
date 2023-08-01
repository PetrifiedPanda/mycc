#include "frontend/Token.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

Token Token_create(TokenKind kind,
                   const StrBuf* spelling,
                   FileLoc file_loc,
                   uint32_t file_idx) {
    assert(spelling);
    assert(file_idx != (uint32_t)-1);
    if (TokenKind_get_spelling(kind).data == NULL) {
        assert(StrBuf_valid(spelling));
    } else {
        assert(!StrBuf_valid(spelling));
    }

    return (Token){
        .kind = kind,
        .spelling = *spelling,
        .loc =
            {
                .file_idx = file_idx,
                .file_loc = file_loc,
            },
    };
}

Token Token_create_copy(TokenKind kind,
                        const StrBuf* spelling,
                        FileLoc file_loc,
                        uint32_t file_idx) {
    assert(spelling);
    assert(StrBuf_valid(spelling));

    return (Token){
        .kind = kind,
        .spelling = StrBuf_copy(spelling),
        .loc =
            {
                .file_idx = file_idx,
                .file_loc = file_loc,
            },
    };
}

StrBuf Token_take_spelling(Token* t) {
    assert(StrBuf_valid(&t->spelling));
    StrBuf spelling = StrBuf_take(&t->spelling);
    return spelling;
}

StrLit Token_take_str_lit(Token* t) {
    assert(t->kind == TOKEN_STRING_LITERAL);
    StrLit res = t->str_lit;
    t->str_lit.contents = StrBuf_null();
    return res;
}

void Token_free(Token* t) {
    assert(t);
    if (t->kind == TOKEN_STRING_LITERAL) {
        StrLit_free(&t->str_lit);
    } else if (t->kind != TOKEN_I_CONSTANT && t->kind != TOKEN_F_CONSTANT) {
        StrBuf_free(&t->spelling);
    }
}

void TokenArr_free_preproc(TokenArr* arr) {
    for (uint32_t i = 0; i < arr->len; ++i) {
        StrBuf_free(&arr->tokens[i].spelling);
    }
    mycc_free(arr->tokens);
}

void TokenArr_free(TokenArr* arr) {
    for (uint32_t i = 0; i < arr->len; ++i) {
        Token_free(&arr->tokens[i]);
    }
    mycc_free(arr->tokens);
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

