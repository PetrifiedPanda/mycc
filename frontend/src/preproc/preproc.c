#include "frontend/preproc/preproc.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

#include "util/macro_util.h"
#include "util/mem.h"
#include "util/file.h"

#include "frontend/token_type.h"

#include "frontend/preproc/preproc_macro.h"
#include "frontend/preproc/preproc_state.h"
#include "frontend/preproc/tokenizer.h"
#include "frontend/preproc/num_parse.h"
#include "frontend/preproc/code_source.h"
#include "frontend/preproc/read_and_tokenize_line.h"

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc include_loc);

static enum token_type keyword_type(const char* spelling);

static void append_terminator_token(struct token_arr* arr);

struct preproc_res preproc(const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = create_preproc_state(path, err);

    if (!preproc_file(&state, path, (struct source_loc){(size_t)-1, {0, 0}})) {
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

static bool preproc_src(struct preproc_state* state, struct code_source* src) {
    while (!code_source_over(src)) {
        const size_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state, src)) {
            return false;
        }

        if (!expand_all_macros(state, &state->res, prev_len, src)) {
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

    struct preproc_state state = create_preproc_state(path, err);

    struct code_source src = create_code_source_string(str, path);

    if (!preproc_src(&state, &src)) {
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

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc include_loc) {
    const size_t current_file_idx = state->file_info.len - 1;
    struct code_source src = create_code_source_file(path,
                                                     state->err,
                                                     current_file_idx,
                                                     include_loc);

    const bool res = preproc_src(state, &src);

    if (!res) {
        // TODO: what to do if closing fails
        // TODO: error may be written twice
        free_code_source(&src);
        return false;
    }
    free_code_source(&src);

    return true;
}

static void free_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
        free_token(it);
    }
    mycc_free(tokens);
}

static void free_preproc_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
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

static bool convert_preproc_token(struct token* t,
                                  const struct arch_type_info* info,
                                  struct preproc_err* err) {
    assert(t);
    assert(info);
    assert(err);
    switch (t->type) {
        case I_CONSTANT: {
            if (str_char_at(&t->spelling, 0) == '\'') {
                struct parse_char_const_res res = parse_char_const(
                    str_get_data(&t->spelling),
                    info);
                if (res.err.type != CHAR_CONST_ERR_NONE) {
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
                if (res.err.type != INT_CONST_ERR_NONE) {
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
        case F_CONSTANT: {
            struct parse_float_const_res res = parse_float_const(
                str_get_data(&t->spelling));
            if (res.err.type != FLOAT_CONST_ERR_NONE) {
                set_preproc_err(err, PREPROC_ERR_FLOAT_CONST, t->loc);
                err->float_const_err = res.err;
                err->constant_spell = t->spelling;
                return false;
            }
            free_str(&t->spelling);
            t->float_val = res.res;
            break;
        }
        case IDENTIFIER:
            t->type = keyword_type(str_get_data(&t->spelling));
            if (t->type != IDENTIFIER) {
                free_str(&t->spelling);
                t->spelling = create_null_str();
            }
            break;
        case STRINGIFY_OP:
        case CONCAT_OP:
            set_preproc_err(err, PREPROC_ERR_MISPLACED_PREPROC_TOKEN, t->loc);
            err->misplaced_preproc_tok = t->type;
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
    for (struct token* t = tokens; t->type != INVALID; ++t) {
        if (!convert_preproc_token(t, info, err)) {
            return false;
        }
    }
    return true;
}

static void append_terminator_token(struct token_arr* arr) {
    arr->tokens = mycc_realloc(arr->tokens, sizeof *arr->tokens * (arr->len + 1));
    arr->tokens[arr->len] = (struct token){
        .type = INVALID,
        .spelling = create_null_str(),
        .loc =
            {
                .file_idx = (size_t)-1,
                .file_loc = {(size_t)-1, (size_t)-1},
            },
    };
}

static inline bool is_spelling(const char* spelling, enum token_type type) {
    const char* expected_spell = get_spelling(type);
    assert(expected_spell != NULL);
    return strcmp(spelling, expected_spell) == 0;
}

static enum token_type keyword_type(const char* spell) {
    for (enum token_type e = KEYWORDS_START; e < KEYWORDS_END; ++e) {
        if (is_spelling(spell, e)) {
            return e;
        }
    }

    return IDENTIFIER;
}

