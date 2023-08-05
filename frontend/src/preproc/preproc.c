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

static bool preproc_impl(PreprocState* state, const ArchTypeInfo* info);

PreprocRes preproc(CStr path, const ArchTypeInfo* info, PreprocErr* err) {
    assert(info);
    assert(err);

    PreprocState state = PreprocState_create(path, err);
    if (err->kind != PREPROC_ERR_NONE) {
        return (PreprocRes){
            .toks = {0},
            .file_info = state.file_info,
        };
    }
    if (!preproc_impl(&state, info)) {
        FileInfo file_info = state.file_info;
        state.file_info = (FileInfo){
            .len = 0,
            .paths = NULL,
        };
        PreprocState_free(&state);
        return (PreprocRes){
            .toks = {0},
            .file_info = file_info,
        };
    }

    PreprocRes res = {
        .toks = state.res,
        .file_info = state.file_info,
    };
    state.res = TokenArr_create_empty();
    state.file_info = (FileInfo){
        .len = 0,
        .paths = NULL,
    };
    PreprocState_free(&state);
    return res;
}

static bool preproc_impl(PreprocState* state, const ArchTypeInfo* info) {
    while (!PreprocState_over(state)) {
        const uint32_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state, info)) {
            return false;
        }

        if (!expand_all_macros(state, &state->res, prev_len, info)) {
            return false;
        }
    }
    if (state->conds_len != 0) {
        PreprocErr_set(state->err,
                       PREPROC_ERR_UNTERMINATED_COND,
                       state->line_info.curr_loc);
        state->err->unterminated_cond_loc = state->conds[state->conds_len - 1]
                                                .loc;
        return false;
    }

    state->res.cap = state->res.len;
    state->res.kinds = mycc_realloc(state->res.kinds,
                                    sizeof *state->res.kinds * state->res.cap);
    state->res.vals = mycc_realloc(state->res.vals,
                                   sizeof *state->res.vals * state->res.cap);
    state->res.locs = mycc_realloc(state->res.locs,
                                   sizeof *state->res.locs * state->res.cap);
    return true;
}

#ifdef MYCC_TEST_FUNCTIONALITY
PreprocRes preproc_string(Str str,
                          Str path,
                          const ArchTypeInfo* info,
                          PreprocErr* err) {
    assert(err);

    PreprocState state = PreprocState_create_string(str, path, err);

    if (!preproc_impl(&state, info)) {
        PreprocState_free(&state);
        return (PreprocRes){
            .toks = {0},
            .file_info =
                {
                    .len = 0,
                    .paths = NULL,
                },
        };
    }

    PreprocRes res = {
        .toks = state.res,
        .file_info = state.file_info,
    };
    state.res = TokenArr_create_empty();
    state.file_info = (FileInfo){
        .len = 0,
        .paths = NULL,
    };
    PreprocState_free(&state);

    return res;
}
#endif // MYCC_TEST_FUNCTIONALITY

static void free_preproc_tokens_from(TokenArr* toks, uint32_t start_idx) {
    for (uint32_t i = start_idx; i < toks->len; ++i) {
        StrBuf_free(&toks->vals[i].spelling);
    }
}

static void free_preproc_tokens(TokenArr* toks) {
    free_preproc_tokens_from(toks, 0);
    mycc_free(toks->kinds);
    mycc_free(toks->vals);
    mycc_free(toks->locs);
}

void PreprocRes_free_preproc_tokens(PreprocRes* res) {
    free_preproc_tokens(&res->toks);
    FileInfo_free(&res->file_info);
}

void PreprocRes_free(PreprocRes* res) {
    TokenArr_free(&res->toks);
    FileInfo_free(&res->file_info);
}

static bool convert_string_literal(TokenVal* val) {
    StrBuf spelling = val->spelling;
    val->str_lit = convert_to_str_lit(&spelling);
    if (val->str_lit.kind == STR_LIT_INCLUDE) {
        StrLit_free(&val->str_lit);
        // TODO: error
        return false;
    }
    return true;
}

static bool convert_preproc_token(uint8_t* kind,
                                  TokenVal* val,
                                  const SourceLoc* loc,
                                  const ArchTypeInfo* info,
                                  PreprocErr* err) {
    assert(val);
    assert(info);
    assert(err);
    switch (*kind) {
        case TOKEN_I_CONSTANT: {
            if (StrBuf_at(&val->spelling, 0) == '\'') {
                ParseCharConstRes res = parse_char_const(
                    StrBuf_as_str(&val->spelling),
                    info);
                if (res.err.kind != CHAR_CONST_ERR_NONE) {
                    PreprocErr_set(err, PREPROC_ERR_CHAR_CONST, *loc);
                    err->char_const_err = res.err;
                    err->constant_spell = val->spelling;
                    return false;
                }
                StrBuf_free(&val->spelling);
                val->val = res.res;
            } else {
                ParseIntConstRes res = parse_int_const(
                    StrBuf_as_str(&val->spelling),
                    info);
                if (res.err.kind != INT_CONST_ERR_NONE) {
                    PreprocErr_set(err, PREPROC_ERR_INT_CONST, *loc);
                    err->int_const_err = res.err;
                    err->constant_spell = val->spelling;
                    return false;
                }
                StrBuf_free(&val->spelling);
                val->val = res.res;
            }
            break;
        }
        case TOKEN_F_CONSTANT: {
            ParseFloatConstRes res = parse_float_const(
                StrBuf_as_str(&val->spelling));
            if (res.err.kind != FLOAT_CONST_ERR_NONE) {
                PreprocErr_set(err, PREPROC_ERR_FLOAT_CONST, *loc);
                err->float_const_err = res.err;
                err->constant_spell = val->spelling;
                return false;
            }
            StrBuf_free(&val->spelling);
            val->val = res.res;
            break;
        }
        case TOKEN_IDENTIFIER:
            *kind = keyword_kind(StrBuf_as_str(&val->spelling));
            if (*kind != TOKEN_IDENTIFIER) {
                StrBuf_free(&val->spelling);
                val->spelling = StrBuf_null();
            }
            break;
        case TOKEN_STRING_LITERAL:
            if (!convert_string_literal(val)) {
                return false;
            }
            break;
        case TOKEN_PP_STRINGIFY:
        case TOKEN_PP_CONCAT:
            PreprocErr_set(err, PREPROC_ERR_MISPLACED_PREPROC_TOKEN, *loc);
            err->misplaced_preproc_tok = *kind;
            return false;
        default:
            break;
    }
    return true;
}

bool convert_preproc_tokens(TokenArr* tokens,
                            const ArchTypeInfo* info,
                            PreprocErr* err) {
    assert(tokens);
    assert(info);
    for (uint32_t i = 0; i < tokens->len; ++i) {
        if (!convert_preproc_token(&tokens->kinds[i],
                                   &tokens->vals[i],
                                   &tokens->locs[i],
                                   info,
                                   err)) {
            free_preproc_tokens_from(tokens, i);
            tokens->len = i;
            return false;
        }
    }
    return true;
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

