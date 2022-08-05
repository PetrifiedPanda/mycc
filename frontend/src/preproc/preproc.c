#include "frontend/preproc/preproc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "util/annotations.h"
#include "util/mem.h"
#include "util/file.h"

#include "frontend/token_type.h"

#include "frontend/preproc/preproc_macro.h"
#include "frontend/preproc/preproc_state.h"
#include "frontend/preproc/tokenizer.h"

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         struct source_loc* include_loc);

static enum token_type keyword_type(const char* spelling);

static void append_terminator_token(struct token_arr* arr);

struct token* preproc(const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = create_preproc_state(err);

    struct source_loc empty_source_loc = {
        .file = NULL,
        .file_loc = {0, 0},
    };
    if (!preproc_file(&state, path, &empty_source_loc)) {
        free_preproc_state(&state);
        return NULL;
    }
    
    struct token_arr res = state.res;
    state.res = (struct token_arr) {
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    free_preproc_state(&state);
    return res.tokens;
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
        ++src->current_line;
    } while (escaped_newline);

    return res;
}

static bool is_preproc_directive(const char* line) {
    const char* it = line;
    while (isspace(*it)) {
        ++it;
    }

    return *it == '#';
}

static bool preproc_statement(struct preproc_state* state,
                              struct code_source* src,
                              struct token_arr* arr);

// TODO: what to do if the line is a preprocessor directive
// Maybe just handle preprocessor directives until we reach an "actual" line
static bool read_and_tokenize_line(struct preproc_state* state,
                                   struct code_source* src) {
    assert(src);

    while (true) {
        char static_buf[PREPROC_LINE_BUF_LEN];
        const size_t prev_curr_line = src->current_line;
        char* line = code_source_read_line(src, static_buf);
        if (line == NULL) {
            return true;
        }

        if (is_preproc_directive(line)) {
            struct token_arr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };

            const bool res = tokenize_line(&arr,
                                           state->err,
                                           line,
                                           prev_curr_line,
                                           src->path,
                                           &src->comment_not_terminated);
            if (line != static_buf) {
                free(line);
            }
            if (!res) {
                return false;
            }

            const bool stat_res = preproc_statement(state, src, &arr);
            free_token_arr(&arr);
            if (!stat_res) {
                return false;
            }
        } else {
            bool res = tokenize_line(&state->res,
                                     state->err,
                                     line,
                                     prev_curr_line,
                                     src->path,
                                     &src->comment_not_terminated);
            if (line != static_buf) {
                free(line);
            }
            if (!res) {
                return false;
            }

            break;
        }
    }

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
                state,
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

static bool preproc_src(struct preproc_state* state, struct code_source* src) {
    while (!code_source_over(src)) {
        const size_t prev_len = state->res.len;
        if (!read_and_tokenize_line(state, src)) {
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

    struct preproc_state state = create_preproc_state(err);

    struct code_source src = {
        .is_file = false,
        .str = str,
        .path = path,
        .current_line = 1,
        .comment_not_terminated = false,
    };

    if (!preproc_src(&state, &src)) {
        free_preproc_state(&state);
        return NULL;
    }
    
    struct token_arr res = state.res;
    state.res = (struct token_arr){
        .len = 0,
        .cap = 0,
        .tokens = NULL,
    };
    free_preproc_state(&state);

    return res.tokens;
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

static bool is_cond_directive(const char* line) {
    const char* it = line;
    while (*it != '\0' && isspace(*it)) {
        ++it;
    }

    if (*it == '\0' || *it != '#') {
        return false;
    }

    ++it;
    while (*it != '\0' && isspace(*it)) {
        ++it;
    }

    char else_dir[] = "else";
    char elif_dir[] = "elif";
    char endif_dir[] = "endif";

    if (*it == '\0') {
        return false;
    } else if (strncmp(it, else_dir, sizeof(else_dir)) == 0
               || strncmp(it, elif_dir, sizeof(elif_dir)) == 0
               || strncmp(it, endif_dir, sizeof(endif_dir))) {
        return true;
    }

    UNREACHABLE();
}

static bool skip_until_next_cond(struct preproc_state* state,
                                 struct code_source* src) {
    while (!code_source_over(src)) {
        char static_buf[PREPROC_LINE_BUF_LEN];
        const size_t prev_curr_line = src->current_line;
        char* line = code_source_read_line(src, static_buf);
        if (is_cond_directive(line)) {
            struct token_arr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };
            const bool res = tokenize_line(&arr,
                                           state->err,
                                           line,
                                           prev_curr_line,
                                           src->path,
                                           &src->comment_not_terminated);

            if (line != static_buf) {
                free(line);
            }

            if (!res) {
                return false;
            }

            bool stat_res = preproc_statement(state, src, &arr);
            free_token_arr(&arr);
            return stat_res;
        }

        if (line != static_buf) {
            free(line);
        }
    }

    return true;
}

static bool handle_preproc_if(struct preproc_state* state,
                              struct code_source* src,
                              bool cond) {
    struct source_loc loc = {
        .file_loc = {src->current_line, 0}, // TODO: might be current_line - 1
        .file = alloc_string_copy(src->path),
    };
    push_preproc_cond(state, loc, cond);

    if (!cond) {
        return skip_until_next_cond(state, src);
    }

    return true;
}

static bool handle_ifdef_ifndef(struct preproc_state* state,
                                struct code_source* src,
                                struct token_arr* arr,
                                bool is_ifndef) {
    assert(arr);
    assert(arr->tokens[0].type == STRINGIFY_OP);
    assert((!is_ifndef && strcmp(arr->tokens[1].spelling, "ifdef") == 0)
           || (is_ifndef && strcmp(arr->tokens[1].spelling, "ifndef")));

    if (arr->len < 3) {
        // TODO: empty ifdef / ifndef error
        return false;
    } else if (arr->len > 3) {
        // TODO: error extra tokens after ifdef / ifndef
        return false;
    }
    if (arr->tokens[2].type != IDENTIFIER) {
        // TODO: error
        return false;
    }

    const char* macro_spell = arr->tokens[2].spelling;
    assert(macro_spell);
    const struct preproc_macro* macro = find_preproc_macro(state, macro_spell);

    const bool cond = is_ifndef ? macro == NULL : macro != NULL;
    return handle_preproc_if(state, src, cond);
}

static bool handle_else_elif(struct preproc_state* state,
                             struct code_source* src,
                             struct token_arr* arr,
                             bool is_else) {
    if (state->conds_len == 0) {
        // TODO: error elif / else without if
        return false;
    }

    struct preproc_cond* curr_if = peek_preproc_cond(state);
    if (curr_if->had_else) {
        // TODO: error elif / else after else
        return false;
    } else if (curr_if->had_true_branch) {
        return skip_until_next_cond(state, src);
    } else if (is_else) {
        curr_if->had_else = true;
        // TODO: just continue
    } else {
        // TODO: evaluate condition
        (void)arr;
    }

    return false;
}

static bool preproc_statement(struct preproc_state* state,
                              struct code_source* src,
                              struct token_arr* arr) {
    assert(arr);
    assert(arr->tokens);
    assert(arr->tokens[0].type == STRINGIFY_OP);
    if (arr->len < 1) {
        // TODO: error
        return false;
    } else if (arr->tokens[1].type != IDENTIFIER) {
        // TODO: error
        return false;
    }

    const char* directive = arr->tokens[1].spelling;
    assert(directive);

    if (strcmp(directive, "if") == 0) {
        // TODO:
    } else if (strcmp(directive, "ifdef") == 0) {
        return handle_ifdef_ifndef(state, src, arr, false);
    } else if (strcmp(directive, "ifndef") == 0) {
        return handle_ifdef_ifndef(state, src, arr, true);
    } else if (strcmp(directive, "define") == 0) {
        struct preproc_macro macro = parse_preproc_macro(arr, state->err);
        if (state->err->type != PREPROC_ERR_NONE) {
            return false;
        }
        char* spelling = arr->tokens[2].spelling;
        arr->tokens[2].spelling = NULL;
        register_preproc_macro(state, spelling, &macro);
    } else if (strcmp(directive, "undef") == 0) {
        if (arr->len < 3) {
            // TODO: error empty undef
            return false;
        } else if (arr->tokens[2].type != IDENTIFIER) {
            // TODO: expected macro name
            return false;
        }

        const char* macro_spell = arr->tokens[2].spelling;
        remove_preproc_macro(state, macro_spell);
    } else if (strcmp(directive, "include") == 0) {
        // TODO:
    } else if (strcmp(directive, "pragma") == 0) {
        // TODO:
    } else if (strcmp(directive, "elif") == 0) {
        return handle_else_elif(state, src, arr, false);
    } else if (strcmp(directive, "else") == 0) {
        return handle_else_elif(state, src, arr, true);
    } else if (strcmp(directive, "endif") == 0) {
        if (state->conds_len == 0) {
            // TODO: error endif without if
            return false;
        }

        pop_preproc_cond(state);
    } else {
        // TODO: invalid preproc directive
        return false;
    }

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

