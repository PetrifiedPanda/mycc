#include "frontend/preproc/preproc.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/log.h"

#include "frontend/preproc/PreprocMacro.h"
#include "frontend/preproc/PreprocState.h"
#include "frontend/preproc/num_parse.h"

#include "read_and_tokenize_line.h"

static TokenKind keyword_kind(Str spelling);

static bool preproc_impl(PreprocState* state, const ArchTypeInfo* info);

PreprocRes preproc(CStr path,
                   uint32_t num_include_dirs,
                   const Str* include_dirs,
                   const ArchTypeInfo* info,
                   PreprocErr* err) {
    assert(info);
    assert(err);
    
    MYCC_TIMER_BEGIN();

    PreprocState state = PreprocState_create(path,
                                             num_include_dirs,
                                             include_dirs,
                                             err);
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
        .toks = state.toks,
        .vals = state.vals,
        .file_info = state.file_info,
    };
    state.toks = PreprocTokenArr_create_empty();
    state.vals = PreprocTokenValList_create_empty();
    state.file_info = (FileInfo){
        .len = 0,
        .paths = NULL,
    };
    PreprocState_free(&state);
    MYCC_TIMER_END("preprocessor");
    return res;
}

static bool preproc_impl(PreprocState* state, const ArchTypeInfo* info) {
    while (!PreprocState_over(state)) {
        const uint32_t prev_len = state->toks.len;
        if (!read_and_tokenize_line(state, info)) {
            return false;
        }

        if (!expand_all_macros(state, &state->toks, prev_len, info)) {
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

    state->toks.cap = state->toks.len;
    state->toks.kinds = mycc_realloc(state->toks.kinds,
                                    sizeof *state->toks.kinds * state->toks.cap);
    state->toks.val_indices = mycc_realloc(state->toks.val_indices,
                                          sizeof *state->toks.val_indices * state->toks.cap);
    state->toks.locs = mycc_realloc(state->toks.locs,
                                   sizeof *state->toks.locs * state->toks.cap);
    return true;
}

#ifdef MYCC_TEST_FUNCTIONALITY

PreprocRes preproc_string(Str str,
                          Str path,
                          const PreprocInitialStrings* initial_strings,
                          uint32_t num_include_dirs,
                          const Str* include_dirs,
                          const ArchTypeInfo* info,
                          PreprocErr* err) {
    assert(err);
    assert(initial_strings);

    PreprocState state = PreprocState_create_string(str,
                                                    path,
                                                    num_include_dirs,
                                                    include_dirs,
                                                    err);
    PreprocTokenValList_insert_initial_strings(&state.vals, initial_strings);

    if (!preproc_impl(&state, info)) {
        PreprocState_free(&state);
        return (PreprocRes){0};
    }

    PreprocRes res = {
        .toks = state.toks,
        .vals = state.vals,
        .file_info = state.file_info,
    };
    state.toks = PreprocTokenArr_create_empty();
    state.vals = PreprocTokenValList_create_empty();
    state.file_info = (FileInfo){
        .len = 0,
        .paths = NULL,
    };
    PreprocState_free(&state);

    return res;
}
#endif // MYCC_TEST_FUNCTIONALITY

void PreprocRes_free_preproc_tokens(PreprocRes* res) {
    PreprocTokenArr_free(&res->toks);
    PreprocTokenValList_free(&res->vals);
    FileInfo_free(&res->file_info);
}

void PreprocRes_free(PreprocRes* res) {
    PreprocTokenArr_free(&res->toks);
    PreprocTokenValList_free(&res->vals);
    FileInfo_free(&res->file_info);
}

// TODO: maybe put in lib (other uses in deserializer I think)
static void* alloc_or_null(size_t bytes) {
    if (bytes == 0) {
        return NULL;
    } else {
        return mycc_alloc(bytes);
    }
}

TokenArr convert_preproc_tokens(PreprocTokenArr* tokens,
                                PreprocTokenValList* vals,
                                const ArchTypeInfo* info,
                                PreprocErr* err) {
    assert(tokens);
    assert(info);
    MYCC_TIMER_BEGIN();
    const uint32_t identifiers_len = IndexedStringSet_len(&vals->identifiers);
    const uint32_t int_consts_len = IndexedStringSet_len(&vals->int_consts);
    const uint32_t float_consts_len = IndexedStringSet_len(&vals->float_consts);
    const uint32_t str_lits_len = IndexedStringSet_len(&vals->str_lits);
    TokenArr res = {
        .len = tokens->len,
        .cap = tokens->cap,
        .kinds = tokens->kinds,
        .val_indices = tokens->val_indices,
        .locs = tokens->locs,
        // Gets initialized when done, so errors work properly
        .identifiers = NULL,
        .int_consts = alloc_or_null(sizeof *res.int_consts * int_consts_len),
        .float_consts = alloc_or_null(sizeof *res.float_consts * float_consts_len),
        .str_lits = alloc_or_null(sizeof *res.str_lits * str_lits_len),
        .identifiers_len = identifiers_len,
        .int_consts_len = int_consts_len,
        .float_consts_len = float_consts_len,
        .str_lits_len = str_lits_len,
    };
    // TODO: if we have identifiers in a set, we should just insert all
    // keywords in the set first 
    // that way we can just check if the token is less than the number of
    // keywords and easily map them
    for (uint32_t i = 0; i < tokens->len; ++i) {
        uint8_t* kind = &tokens->kinds[i];
        switch (*kind) {
            case TOKEN_IDENTIFIER: {
                Str spelling = IndexedStringSet_get(&vals->identifiers, tokens->val_indices[i]);
                *kind = keyword_kind(spelling);
                break;
            }
            case TOKEN_PP_STRINGIFY:
            case TOKEN_PP_CONCAT:
                PreprocErr_set(err, PREPROC_ERR_MISPLACED_PREPROC_TOKEN, tokens->locs[i]);
                err->misplaced_preproc_tok = *kind;
                // TODO: free
                return (TokenArr){0};
        }
    }
    for (uint32_t i = 0; i < int_consts_len; ++i) {
        Str spelling = IndexedStringSet_get(&vals->int_consts, i);
        if (Str_at(spelling, 0) == '\'') {
            ParseCharConstRes char_const = parse_char_const(spelling, info);
            if (char_const.err.kind != CHAR_CONST_ERR_NONE) {
                // TODO: find source loc
                PreprocErr_set(err, PREPROC_ERR_CHAR_CONST, (SourceLoc){0});
                err->char_const_err = char_const.err;
                err->constant_spell = spelling;
                // TODO: free
                return (TokenArr){0};
            }
            res.int_consts[i] = char_const.res;
        } else {
            ParseIntConstRes int_const = parse_int_const(
                spelling,
                info);
            if (int_const.err.kind != INT_CONST_ERR_NONE) {
                // TODO: find source loc
                PreprocErr_set(err, PREPROC_ERR_INT_CONST, (SourceLoc){0});
                err->int_const_err = int_const.err;
                err->constant_spell = spelling;
                // TODO: free
                return (TokenArr){0};
            }
            res.int_consts[i] = int_const.res;
        }
    }
    for (uint32_t i = 0; i < float_consts_len; ++i) {
        Str spelling = IndexedStringSet_get(&vals->float_consts, i);
        ParseFloatConstRes float_const = parse_float_const(spelling);
        if (float_const.err.kind != FLOAT_CONST_ERR_NONE) {
            // TODO: find source loc
            PreprocErr_set(err, PREPROC_ERR_FLOAT_CONST, (SourceLoc){0});
            err->float_const_err = float_const.err;
            err->constant_spell = spelling;
            // TODO: free
            return (TokenArr){0};
        }
        res.float_consts[i] = float_const.res; 
    }
    for (uint32_t i = 0; i < str_lits_len; ++i) {
        Str spelling = IndexedStringSet_get(&vals->str_lits, i);
        StrLit* str_lit = &res.str_lits[i];
        *str_lit = convert_to_str_lit(spelling);
        if (str_lit->kind == STR_LIT_INCLUDE) {
            StrLit_free(str_lit);
            // TODO: error
            // TODO: free
            return (TokenArr){0};
        }
    }
    res.identifiers = IndexedStringSet_take(&vals->identifiers);
    IndexedStringSet_free(&vals->float_consts);
    IndexedStringSet_free(&vals->int_consts);
    IndexedStringSet_free(&vals->str_lits);
    *tokens = (PreprocTokenArr){0};
    *vals = (PreprocTokenValList){0};
    MYCC_TIMER_END("converting preproc tokens");
    return res;
}

static inline bool is_spelling(Str spelling, TokenKind type) {
    Str expected_spell = TokenKind_get_spelling(type);
    assert(expected_spell.data != NULL);
    return Str_eq(spelling, expected_spell);
}

static TokenKind keyword_kind(Str spell) {
    assert(spell.len != 0);
    for (TokenKind e = TOKEN_KEYWORDS_START; e < TOKEN_KEYWORDS_END; ++e) {
        if (is_spelling(spell, e)) {
            return e;
        }
    }

    return TOKEN_IDENTIFIER;
}

