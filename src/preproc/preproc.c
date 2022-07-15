#include "preproc/preproc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "token_type.h"

#include "util/mem.h"
#include "util/file.h"

#include "preproc/preproc_macro.h"
#include "preproc/preproc_state.h"
#include "preproc/tokenizer.h"

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc* include_loc);

static enum token_type keyword_type(const char* spelling);

static void append_terminator_token(struct token_arr* arr);

struct token* preproc(const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = {
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .err = err,
    };

    struct source_loc empty_source_loc = {
        .file = NULL,
        .file_loc = {0, 0},
    };
    if (!preproc_file(&state, path, &empty_source_loc)) {
        for (size_t i = 0; i < state.res.len; ++i) {
            free_token(&state.res.tokens[i]);
        }
        free(state.res.tokens);
        return NULL;
    }

    append_terminator_token(&state.res);

    return state.res.tokens;
}

enum {
    PREPROC_LINE_BUF_LEN = 200
};

struct code_source {
    bool is_file;
    union {
        FILE* file;
        const char* str;
    };
    const char* path;
    size_t current_line;
    bool comment_not_terminated;
};

static bool code_source_over(struct code_source* src) {
    if (src->is_file) {
        return feof(src->file);
    } else {
        return *src->str == '\0';
    }
}

static void string_read_line(const char** str,
                             char** res,
                             size_t* res_len,
                             char* static_buf,
                             size_t static_buf_len) {

    const char* start = *str;
    const char* it = *str;
    if (*res_len < static_buf_len) {
        *res = static_buf;
        bool use_dyn_buf = false;
        while (*it != '\n' && *it != '\0') {
            ++it;
            if (*res_len == static_buf_len - 1) {
                use_dyn_buf = true;
                break;
            }
        }

        if (!use_dyn_buf) {
            if (it == start && *res_len == 0) {
                *res = NULL;
            } else {
                const size_t len = it - start;
                *str = *it == '\0' ? it : it + 1;
                memcpy(static_buf + *res_len, start, sizeof(char) * len);
                *res_len += len;
                static_buf[len] = '\0';
            }
            return;
        }
    }

    while (*it != '\n' && *it != '\0') {
        ++it;
    }

    *str = *it == '\0' ? it : it + 1;
    const size_t len = it - start;
    const size_t prev_res_len = *res_len;
    *res_len += len;
    if (len == 0) {
        return;
    }
    *res = xrealloc(*res, sizeof(char) * (*res_len + 1));
    memcpy(*res + prev_res_len, start, len * sizeof(char));
    (*res)[*res_len] = '\0';
}

static char* code_source_read_line(struct code_source* src,
                                   char static_buf[PREPROC_LINE_BUF_LEN]) {
    char* res = NULL;
    bool escaped_newline = false;
    size_t len = 0;
    do {
        if (src->is_file) {
            file_read_line(src->file,
                           &res,
                           &len,
                           static_buf,
                           PREPROC_LINE_BUF_LEN);
        } else {
            string_read_line(&src->str,
                             &res,
                             &len,
                             static_buf,
                             PREPROC_LINE_BUF_LEN);
        }

        if (res != NULL && len > 0) {
            escaped_newline = res[len - 1] == '\\';
            // TODO: newlines not contained when escaped newline is found
        }
    } while (escaped_newline);

    return res;
}

// TODO: what to do if the line is a preprocessor directive
// Maybe just handle preprocessor directives until we reach an "actual" line
static bool read_and_tokenize_line(struct preproc_state* state,
                                   struct code_source* src) {
    assert(src);

    char static_buf[PREPROC_LINE_BUF_LEN];
    char* line = code_source_read_line(src, static_buf);
    if (line == NULL) {
        return true;
    }
    bool res = tokenize_line(&state->res,
                             state->err,
                             line,
                             src->current_line,
                             src->path,
                             &src->comment_not_terminated);
    if (line != static_buf) {
        free(line);
    }
    if (!res) {
        return false;
    }
    ++src->current_line;
    return true;
}

static const struct token* find_macro_end(struct preproc_state* state,
                                          const struct token* macro_start,
                                          struct code_source* src) {
    const struct token* it = macro_start;
    assert(it->type == IDENTIFIER);
    ++it;
    assert(it->type == LBRACKET);
    ++it;

    size_t open_bracket_count = 0;
    while (!code_source_over(src)
           && (open_bracket_count != 0 || it->type != RBRACKET)) {
        while (!code_source_over(src)
               && it == state->res.tokens + state->res.len) {
            if (!read_and_tokenize_line(state, src)) {
                return NULL;
            }
        }

        if (code_source_over(src) && it == state->res.tokens + state->res.len) {
            break;
        }

        if (it->type == LBRACKET) {
            ++open_bracket_count;
        } else if (it->type == RBRACKET) {
            --open_bracket_count;
        }
        ++it;
    }

    if (it->type != RBRACKET) {
        set_preproc_err_copy(state->err,
                             PREPROC_ERR_UNTERMINATED_MACRO,
                             &macro_start->loc);
        return NULL;
    }
    return it;
}

static bool expand_all_macros(struct preproc_state* state,
                              size_t start,
                              struct code_source* src) {
    bool no_incr = false;
    for (size_t i = start; i < state->res.len; ++i) {
        if (no_incr) {
            --i;
            no_incr = false;
        }

        const struct token* curr = &state->res.tokens[i];
        if (curr->type == IDENTIFIER) {
            const struct preproc_macro* macro = find_preproc_macro(
                curr->spelling);
            if (macro != NULL) {
                const struct token* macro_end;
                if (macro->is_func_macro) {
                    macro_end = find_macro_end(state, curr, src);
                    if (state->err != PREPROC_ERR_NONE) {
                        return false;
                    } else if (macro_end == NULL) {
                        continue;
                    }
                } else {
                    macro_end = NULL;
                }
                if (!expand_preproc_macro(state, macro, i, macro_end)) {
                    return false;
                }
                // need to continue at the start of the macro expansion
                no_incr = true;
            }
        }
    }

    return true;
}

static bool preproc_statement(struct preproc_state* res,
                              size_t line_start,
                              struct code_source* src);

static bool preproc_src(struct preproc_state* state, struct code_source* src) {
    while (!code_source_over(src)) {
        const size_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state, src)) {
            return false;
        }

        if (state->res.len != prev_len
            && state->res.tokens[prev_len].type == STRINGIFY_OP
            && !preproc_statement(state, prev_len, src)) {
            return false;
        }

        expand_all_macros(state, prev_len, src);
    }

    append_terminator_token(&state->res);
    return true;
}

struct token* preproc_string(const char* str,
                             const char* path,
                             struct preproc_err* err) {
    assert(err);

    struct preproc_state state = {
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .err = err,
    };

    struct code_source src = {
        .is_file = false,
        .str = str,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };

    if (!preproc_src(&state, &src)) {
        for (size_t i = 0; i < state.res.len; ++i) {
            free_token(&state.res.tokens[i]);
        }
        free(state.res.tokens);
        return NULL;
    }

    return state.res.tokens;
}

static void file_err(struct preproc_err* err,
                     const char* path,
                     struct source_loc* include_loc,
                     bool open_fail);

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc* include_loc) {
    FILE* file = fopen(path, "r");
    if (!file) {
        file_err(state->err, path, include_loc, true);
        return false;
    }

    struct code_source src = {
        .is_file = true,
        .file = file,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };

    const bool res = preproc_src(state, &src);

    if (!res) {
        // TODO: what to do if closing fails
        fclose(file);
        return false;
    }

    if (fclose(file) != 0) {
        file_err(state->err, path, include_loc, false);
        return false;
    }

    return true;
}

static void file_err(struct preproc_err* err,
                     const char* path,
                     struct source_loc* include_loc,
                     bool open_fail) {
    assert(path);

    set_preproc_err(err, PREPROC_ERR_FILE_FAIL, include_loc);
    err->fail_file = alloc_string_copy(path);
    err->open_fail = open_fail;
}

void free_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
        free_token(it);
    }
    free(tokens);
}

void convert_preproc_tokens(struct token* tokens) {
    assert(tokens);
    for (struct token* t = tokens; t->type != INVALID; ++t) {
        assert(t->type != STRINGIFY_OP && t->type != CONCAT_OP);
        if (t->type == IDENTIFIER) {
            t->type = keyword_type(t->spelling);
            if (t->type != IDENTIFIER) {
                free(t->spelling);
                t->spelling = NULL;
            }
        }
    }
}

static void append_terminator_token(struct token_arr* arr) {
    arr->tokens = xrealloc(arr->tokens, sizeof(struct token) * (arr->len + 1));
    arr->tokens[arr->len] = (struct token){
        .type = INVALID,
        .spelling = NULL,
        .loc =
            {
                .file = NULL,
                .file_loc = {(size_t)-1, (size_t)-1},
            },
    };
}

static bool preproc_statement(struct preproc_state* state,
                              size_t line_start,
                              struct code_source* src) {
    assert(src != NULL);
    assert(state->res.tokens[line_start].type == STRINGIFY_OP);
    (void)state;
    (void)line_start;
    (void)src;
    // TODO:
    return false;
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

