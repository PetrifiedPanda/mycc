#include "frontend/preproc/preproc.h"

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#include "util/macro_util.h"
#include "util/mem.h"

#include "frontend/preproc/PreprocMacro.h"
#include "frontend/preproc/PreprocState.h"
#include "frontend/preproc/num_parse.h"

#include "tokenizer.h"
#include "read_and_tokenize_line.h"

static TokenKind keyword_kind(Str spelling);

static void append_terminator_token(TokenArr* arr);

static bool preproc_impl(PreprocState* state, const ArchTypeInfo* info);

PreprocRes preproc(CStr path, const ArchTypeInfo* info, PreprocErr* err) {
    assert(info);
    assert(err);

    PreprocState state = PreprocState_create(path, err);
    if (err->kind != PREPROC_ERR_NONE) {
        return (PreprocRes){
            .toks = NULL,
            .file_info = state.file_info,
        };
    }
    if (!preproc_impl(&state, info)) {
        FileInfo info = state.file_info;
        state.file_info = (FileInfo){
            .len = 0,
            .paths = NULL,
        };
        PreprocState_free(&state);
        return (PreprocRes){
            .toks = NULL,
            .file_info = info,
        };
    }

    PreprocRes res = {
        .toks = state.res.tokens,
        .file_info = state.file_info,
    };
    state.res = (TokenArr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    state.file_info = (FileInfo){
        .len = 0,
        .paths = NULL,
    };
    PreprocState_free(&state);
    return res;
}

static bool preproc_impl(PreprocState* state, const ArchTypeInfo* info) {
    while (!PreprocState_over(state)) {
        const size_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state, info)) {
            return false;
        }

        if (!expand_all_macros(state, &state->res, prev_len, info)) {
            return false;
        }
    }
    if (state->conds_len != 0) {
        PreprocErr_set(state->err, PREPROC_ERR_UNTERMINATED_COND, state->line_info.curr_loc);
        state->err->unterminated_cond_loc = state->conds[state->conds_len - 1].loc;
        return false;
    }
    append_terminator_token(&state->res);
    return true;
}

#ifdef MYCC_TEST_FUNCTIONALITY
PreprocRes preproc_string(Str str, Str path, const ArchTypeInfo* info, PreprocErr* err) {
    assert(err);

    PreprocState state = PreprocState_create_string(str, path, err);

    if (!preproc_impl(&state, info)) {
        PreprocState_free(&state);
        return (PreprocRes){
            .toks = NULL,
            .file_info =
                {
                    .len = 0,
                    .paths = NULL,
                },
        };
    }

    PreprocRes res = {
        .toks = state.res.tokens,
        .file_info = state.file_info,
    };
    state.res = (TokenArr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    state.file_info = (FileInfo){
        .len = 0,
        .paths = NULL,
    };
    PreprocState_free(&state);

    return res;
}
#endif // MYCC_TEST_FUNCTIONALITY

static void free_tokens(Token* tokens) {
    for (Token* it = tokens; it->kind != TOKEN_INVALID; ++it) {
        Token_free(it);
    }
    mycc_free(tokens);
}

static void free_preproc_tokens(Token* tokens) {
    for (Token* it = tokens; it->kind != TOKEN_INVALID; ++it) {
        StrBuf_free(&it->spelling);
    }
    mycc_free(tokens);
}

void PreprocRes_free_preproc_tokens(PreprocRes* res) {
    free_preproc_tokens(res->toks);
    FileInfo_free(&res->file_info);
}

void PreprocRes_free(PreprocRes* res) {
    if (res->toks) {
        free_tokens(res->toks);
    }
    FileInfo_free(&res->file_info);
}

static bool convert_string_literal(Token* t) {
    StrBuf spelling = t->spelling;
    t->str_lit = convert_to_str_lit(&spelling);
    if (t->str_lit.kind == STR_LIT_INCLUDE) {
        StrLit_free(&t->str_lit);
        // TODO: error
        return false;
    }
    return true;
}

static bool convert_preproc_token(Token* t,
                                  const ArchTypeInfo* info,
                                  PreprocErr* err) {
    assert(t);
    assert(info);
    assert(err);
    switch (t->kind) {
        case TOKEN_I_CONSTANT: {
            if (StrBuf_at(&t->spelling, 0) == '\'') {
                ParseCharConstRes res = parse_char_const(StrBuf_as_str(&t->spelling), info);
                if (res.err.kind != CHAR_CONST_ERR_NONE) {
                    PreprocErr_set(err, PREPROC_ERR_CHAR_CONST, t->loc);
                    err->char_const_err = res.err;
                    err->constant_spell = t->spelling;
                    return false;
                }
                StrBuf_free(&t->spelling);
                t->val = res.res;
            } else {
                ParseIntConstRes res = parse_int_const(StrBuf_as_str(&t->spelling), info);
                if (res.err.kind != INT_CONST_ERR_NONE) {
                    PreprocErr_set(err, PREPROC_ERR_INT_CONST, t->loc);
                    err->int_const_err = res.err;
                    err->constant_spell = t->spelling;
                    return false;
                }
                StrBuf_free(&t->spelling);
                t->val = res.res;
            }
            break;
        }
        case TOKEN_F_CONSTANT: {
            ParseFloatConstRes res = parse_float_const(StrBuf_as_str(&t->spelling));
            if (res.err.kind != FLOAT_CONST_ERR_NONE) {
                PreprocErr_set(err, PREPROC_ERR_FLOAT_CONST, t->loc);
                err->float_const_err = res.err;
                err->constant_spell = t->spelling;
                return false;
            }
            StrBuf_free(&t->spelling);
            t->val = res.res;
            break;
        }
        case TOKEN_IDENTIFIER:
            t->kind = keyword_kind(StrBuf_as_str(&t->spelling));
            if (t->kind != TOKEN_IDENTIFIER) {
                StrBuf_free(&t->spelling);
                t->spelling = StrBuf_null();
            }
            break;
        case TOKEN_STRING_LITERAL:
            if (!convert_string_literal(t)) {
                return false;
            }
            break;
        case TOKEN_PP_STRINGIFY:
        case TOKEN_PP_CONCAT:
            PreprocErr_set(err, PREPROC_ERR_MISPLACED_PREPROC_TOKEN, t->loc);
            err->misplaced_preproc_tok = t->kind;
            return false;
        default:
            break;
    }
    return true;
}

bool convert_preproc_tokens(Token* tokens,
                            const ArchTypeInfo* info,
                            PreprocErr* err) {
    assert(tokens);
    assert(info);
    for (Token* t = tokens; t->kind != TOKEN_INVALID; ++t) {
        if (!convert_preproc_token(t, info, err)) {
            return false;
        }
    }
    return true;
}

static void append_terminator_token(TokenArr* arr) {
    arr->tokens = mycc_realloc(arr->tokens, sizeof *arr->tokens * (arr->len + 1));
    arr->tokens[arr->len] = (Token){
        .kind = TOKEN_INVALID,
        .spelling = StrBuf_null(),
        .loc =
            {
                .file_idx = (size_t)-1,
                .file_loc = {(size_t)-1, (size_t)-1},
            },
    };
}

static inline bool is_spelling(Str spelling, TokenKind type) {
    Str expected_spell = TokenKind_get_spelling(type);
    assert(expected_spell.data != NULL);
    return Str_eq(spelling, expected_spell);
}

static TokenKind keyword_kind(Str spell) {
    for (TokenKind e = TOKEN_KEYWORDS_START; e < TOKEN_KEYWORDS_END; ++e) {
        if (is_spelling(spell, e)) {
            return e;
        }
    }

    return TOKEN_IDENTIFIER;
}

