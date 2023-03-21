#include "frontend/preproc/preproc.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#include "util/macro_util.h"
#include "util/mem.h"
#include "util/file.h"

#include "frontend/preproc/preproc_macro.h"
#include "frontend/preproc/preproc_state.h"
#include "frontend/preproc/tokenizer.h"
#include "frontend/preproc/num_parse.h"
#include "frontend/preproc/read_and_tokenize_line.h"

static enum token_kind keyword_kind(const char* spelling);

static void append_terminator_token(struct token_arr* arr);

static bool preproc_impl(struct preproc_state* state);

struct preproc_res preproc(const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = create_preproc_state(path, err);
    if (err->kind != PREPROC_ERR_NONE) {
        return (struct preproc_res){
            .toks = NULL,
            .file_info = state.file_info,
        };
    }
    if (!preproc_impl(&state)) {
        struct file_info info = state.file_info;
        state.file_info = (struct file_info){
            .len = 0,
            .paths = NULL,
        };
        free_preproc_state(&state);
        return (struct preproc_res){
            .toks = NULL,
            .file_info = info,
        };
    }

    struct preproc_res res = {
        .toks = state.res.tokens,
        .file_info = state.file_info,
    };
    state.res = (struct token_arr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    state.file_info = (struct file_info){
        .len = 0,
        .paths = NULL,
    };
    free_preproc_state(&state);
    return res;
}

static bool preproc_impl(struct preproc_state* state) {
    while (!preproc_state_over(state)) {
        const size_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state)) {
            return false;
        }

        if (!expand_all_macros(state, &state->res, prev_len)) {
            return false;
        }
    }
    append_terminator_token(&state->res);
    return true;
}

#ifdef MYCC_TEST_FUNCTIONALITY
struct preproc_res preproc_string(const char* str,
                                  const char* path,
                                  struct preproc_err* err) {
    assert(err);

    struct preproc_state state = create_preproc_state_string(str, path, err);

    if (!preproc_impl(&state)) {
        free_preproc_state(&state);
        return (struct preproc_res){
            .toks = NULL,
            .file_info =
                {
                    .len = 0,
                    .paths = NULL,
                },
        };
    }

    struct preproc_res res = {
        .toks = state.res.tokens,
        .file_info = state.file_info,
    };
    state.res = (struct token_arr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    state.file_info = (struct file_info){
        .len = 0,
        .paths = NULL,
    };
    free_preproc_state(&state);

    return res;
}
#endif // MYCC_TEST_FUNCTIONALITY

static void free_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->kind != TOKEN_INVALID; ++it) {
        free_token(it);
    }
    mycc_free(tokens);
}

static void free_preproc_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->kind != TOKEN_INVALID; ++it) {
        free_str(&it->spelling);
    }
    mycc_free(tokens);
}

void free_preproc_res_preproc_tokens(struct preproc_res* res) {
    free_preproc_tokens(res->toks);
    free_file_info(&res->file_info);
}

void free_preproc_res(struct preproc_res* res) {
    if (res->toks) {
        free_tokens(res->toks);
    }
    free_file_info(&res->file_info);
}

static void convert_string_literal(struct token* t) {
    struct str spelling = t->spelling;
    // TODO: change this assert when '<' '>' literals are there
    assert(str_get_data(&spelling)[str_len(&spelling) - 1] == '"');
    str_pop_back(&spelling);
    const char* data = str_get_data(&spelling);
    enum str_lit_kind kind;
    size_t chars_to_remove;
    switch (data[0]) {
        case '"':
            kind = STR_LIT_DEFAULT;
            chars_to_remove = 1;
            break;
        case 'u':
            if (data[1] == '8') {
                assert(data[2] == '"');
                kind = STR_LIT_U8;
                chars_to_remove = 3;
            } else {
                assert(data[1] == '"');
                kind = STR_LIT_LOWER_U;
                chars_to_remove = 2;
            }
            break;
        case 'U':
            kind = STR_LIT_UPPER_U;
            chars_to_remove = 2;
            break;
        case 'L':
            kind = STR_LIT_L;
            chars_to_remove = 2;
            break;
        default:
            UNREACHABLE();
    }
    // TODO: still need to convert escape sequences
    str_remove_front(&spelling, chars_to_remove);
    str_shrink_to_fit(&spelling);
    t->str_lit = create_str_lit(kind, &spelling);
}

static bool convert_preproc_token(struct token* t,
                                  const struct arch_type_info* info,
                                  struct preproc_err* err) {
    assert(t);
    assert(info);
    assert(err);
    switch (t->kind) {
        case TOKEN_I_CONSTANT: {
            if (str_char_at(&t->spelling, 0) == '\'') {
                struct parse_char_const_res res = parse_char_const(
                    str_get_data(&t->spelling),
                    info);
                if (res.err.kind != CHAR_CONST_ERR_NONE) {
                    set_preproc_err(err, PREPROC_ERR_CHAR_CONST, t->loc);
                    err->char_const_err = res.err;
                    err->constant_spell = t->spelling;
                    return false;
                }
                free_str(&t->spelling);
                t->int_val = res.res;
            } else {
                struct parse_int_const_res res = parse_int_const(
                    str_get_data(&t->spelling),
                    info);
                if (res.err.kind != INT_CONST_ERR_NONE) {
                    set_preproc_err(err, PREPROC_ERR_INT_CONST, t->loc);
                    err->int_const_err = res.err;
                    err->constant_spell = t->spelling;
                    return false;
                }
                free_str(&t->spelling);
                t->int_val = res.res;
            }
            break;
        }
        case TOKEN_F_CONSTANT: {
            struct parse_float_const_res res = parse_float_const(
                str_get_data(&t->spelling));
            if (res.err.kind != FLOAT_CONST_ERR_NONE) {
                set_preproc_err(err, PREPROC_ERR_FLOAT_CONST, t->loc);
                err->float_const_err = res.err;
                err->constant_spell = t->spelling;
                return false;
            }
            free_str(&t->spelling);
            t->float_val = res.res;
            break;
        }
        case TOKEN_IDENTIFIER:
            t->kind = keyword_kind(str_get_data(&t->spelling));
            if (t->kind != TOKEN_IDENTIFIER) {
                free_str(&t->spelling);
                t->spelling = create_null_str();
            }
            break;
        case TOKEN_STRING_LITERAL:
            convert_string_literal(t);
            break;
        case TOKEN_PP_STRINGIFY:
        case TOKEN_PP_CONCAT:
            set_preproc_err(err, PREPROC_ERR_MISPLACED_PREPROC_TOKEN, t->loc);
            err->misplaced_preproc_tok = t->kind;
            return false;
        default:
            break;
    }
    return true;
}

bool convert_preproc_tokens(struct token* tokens,
                            const struct arch_type_info* info,
                            struct preproc_err* err) {
    assert(tokens);
    assert(info);
    for (struct token* t = tokens; t->kind != TOKEN_INVALID; ++t) {
        if (!convert_preproc_token(t, info, err)) {
            return false;
        }
    }
    return true;
}

static void append_terminator_token(struct token_arr* arr) {
    arr->tokens = mycc_realloc(arr->tokens, sizeof *arr->tokens * (arr->len + 1));
    arr->tokens[arr->len] = (struct token){
        .kind = TOKEN_INVALID,
        .spelling = create_null_str(),
        .loc =
            {
                .file_idx = (size_t)-1,
                .file_loc = {(size_t)-1, (size_t)-1},
            },
    };
}

static inline bool is_spelling(const char* spelling, enum token_kind type) {
    const char* expected_spell = get_token_kind_spelling(type);
    assert(expected_spell != NULL);
    return strcmp(spelling, expected_spell) == 0;
}

static enum token_kind keyword_kind(const char* spell) {
    for (enum token_kind e = TOKEN_KEYWORDS_START; e < TOKEN_KEYWORDS_END; ++e) {
        if (is_spelling(spell, e)) {
            return e;
        }
    }

    return TOKEN_IDENTIFIER;
}

