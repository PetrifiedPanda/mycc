#include "frontend/Token.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

Token create_token(TokenKind kind,
                   const Str* spelling,
                   FileLoc file_loc,
                   size_t file_idx) {
    assert(spelling);
    assert(file_idx != (size_t)-1);
    if (get_token_kind_spelling(kind) == NULL) {
        assert(Str_is_valid(spelling));
    } else {
        assert(!Str_is_valid(spelling));
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

Token create_token_copy(TokenKind kind,
                        const Str* spelling,
                        FileLoc file_loc,
                        size_t file_idx) {
    assert(spelling);
    assert(Str_is_valid(spelling));

    return (Token){
        .kind = kind,
        .spelling = Str_copy(spelling),
        .loc =
            {
                .file_idx = file_idx,
                .file_loc = file_loc,
            },
    };
}

Str token_take_spelling(Token* t) {
    assert(Str_is_valid(&t->spelling));
    Str spelling = Str_take(&t->spelling);
    return spelling;
}

StrLit token_take_str_lit(Token* t) {
    assert(t->kind == TOKEN_STRING_LITERAL);
    StrLit res = t->str_lit;
    t->str_lit.contents = Str_create_null();
    return res;
}

void free_token(Token* t) {
    assert(t);
    if (t->kind == TOKEN_STRING_LITERAL) {
        free_str_lit(&t->str_lit);
    } else if (t->kind != TOKEN_I_CONSTANT && t->kind != TOKEN_F_CONSTANT) {
        Str_free(&t->spelling);
    }
}

const char* get_token_kind_spelling(TokenKind kind) {
    switch (kind) {
#define TOKEN_MACRO(kind, str)                                                 \
    case kind:                                                                 \
        return str;
#include "frontend/TokenKind.inc"
#undef TOKEN_MACRO
    }
    UNREACHABLE();
}

const char* get_token_kind_str(TokenKind kind) {
    switch (kind) {
#define TOKEN_MACRO(kind, str)                                                 \
    case kind:                                                                 \
        return #kind;
#include "frontend/TokenKind.inc"
#undef TOKEN_MACRO
    }
    UNREACHABLE();
}

