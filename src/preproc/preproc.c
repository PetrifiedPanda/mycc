#include "preproc/preproc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "token_type.h"

#include "util/mem.h"

#include "preproc/preproc_macro.h"
#include "preproc/preproc_state.h"
#include "preproc/tokenizer.h"

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         const char* include_file,
                         struct source_location include_loc);

static enum token_type keyword_type(const char* spelling);

static void append_terminator_token(struct token** tokens, size_t len);
void convert_preproc_tokens(struct token* tokens);

struct token* preproc(const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = {
        .len = 0,
        .cap = 0,
        .tokens = NULL,
        .err = err,
    };

    if (!preproc_file(&state, path, NULL, (struct source_location){0, 0})) {
        for (size_t i = 0; i < state.len; ++i) {
            free_token(&state.tokens[i]);
        }
        free(state.tokens);
        return NULL;
    }

    append_terminator_token(&state.tokens, state.len);

    return state.tokens;
}

enum {
    PREPROC_LINE_BUF_LEN = 200
};

static char* file_read_line(FILE* file, char static_buf[PREPROC_LINE_BUF_LEN], bool* escaped_newline) {
    assert(escaped_newline);

    size_t i = 0;
    int c;
    while ((c = getc(file)) != '\n' && c != EOF) {
       static_buf[i] = (char)c;
        ++i;
        if (i == PREPROC_LINE_BUF_LEN - 1) {
            break;
        }
    }

    if (i == 0 && c == EOF) { // only EOF read
        return NULL;
    }

    char* res = static_buf;
    
    if (c != '\n' && c != EOF) {
        size_t len = PREPROC_LINE_BUF_LEN * 2;
        char* dyn_buf = xmalloc(sizeof(char) * len);
        memcpy(dyn_buf, static_buf, (PREPROC_LINE_BUF_LEN - 1) * sizeof(char));

        while ((c = getc(file)) != '\n' && c != EOF) {
            if (i == len - 1) {
                grow_alloc((void**)&dyn_buf, &len, sizeof(char));
            }

            dyn_buf[i] = (char)c;
            ++i;
        }

        res = dyn_buf;
    }
    if (i > 0) { 
        *escaped_newline = res[i - 1] == '\\';
    }
    res[i] = '\0';
    return res;
}

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

static char* code_source_read_line(struct code_source* src, char static_buf[PREPROC_LINE_BUF_LEN], bool* escaped_newline) {
    char* res;
    if (src->is_file) {
        res = file_read_line(src->file, static_buf, escaped_newline);    
    } else {
        const char* start = src->str;
        const char* it = src->str;
        while (*it != '\n' && *it != '\0') {
            ++it;
        }
        
        src->str = *it == '\0' ? it : it + 1;
            
        const size_t len = it - start;
        res = len != 0 ? xmalloc(sizeof(char) * (len + 1)) : NULL;
        memcpy(res, start, len * sizeof(char));
        if (res != NULL) {
            res[len] = '\0';
            *escaped_newline = res[len - 1] == '\\';
        }
    }
    return res; 
}

// TODO: what to do if the line is a preprocessor directive
// Maybe just handle preprocessor directives until we reach an "actual" line
static bool read_and_tokenize_line(struct preproc_state* state,
                                   struct code_source* src) {
    assert(src);

    bool escaped_newline = false;    
    do {
        char static_buf[PREPROC_LINE_BUF_LEN];
        char* line = code_source_read_line(src, static_buf, &escaped_newline);
        if (line == NULL) {
            return true;
        }
        bool res = tokenize_line(state,
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
    } while (escaped_newline);

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
    while (!code_source_over(src) && (open_bracket_count != 0 || it->type != RBRACKET)) {
        while (!code_source_over(src) && it == state->tokens + state->len) {
            if (!read_and_tokenize_line(state, src)) {
                return NULL;
            }
        }

        if (code_source_over(src) && it == state->tokens + state->len) {
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
                             macro_start->file, 
                             macro_start->source_loc);
        return NULL;
    }
    return it;
}

static bool expand_all_macros(struct preproc_state* state, size_t start, struct code_source* src) {
    bool no_incr = false;
    for (size_t i = start; i < state->len; ++i) {
        if (no_incr) {
            --i;
            no_incr = false;
        }

        const struct token* curr = &state->tokens[i];
        if (curr->type == IDENTIFIER) {
            const struct preproc_macro* macro = find_preproc_macro(curr->spelling);
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
                no_incr = false;
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
        const size_t prev_len = state->len;
        if (!read_and_tokenize_line(state, src)) {
            return false;
        }

        if (state->len != prev_len 
            && state->tokens[prev_len].type == STRINGIFY_OP
            && !preproc_statement(state, prev_len, src)) { 
            return false;
        }

        expand_all_macros(state, prev_len, src);
    }

    append_terminator_token(&state->tokens, state->len);
    return true;
}

struct token* preproc_string(const char* str, const char* path, struct preproc_err* err) {
    assert(err);

    struct preproc_state state = {
        .len = 0,
        .cap = 0,
        .tokens = NULL,
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
        for (size_t i = 0; i < state.len; ++i) {
            free_token(&state.tokens[i]);
        }
        free(state.tokens);
        return NULL; 
    }

    return state.tokens;
}

static void file_err(struct preproc_err* err,
                     const char* path,
                     const char* include_file,
                     struct source_location include_loc,
                     bool open_fail);

static bool preproc_file(struct preproc_state* state,
                         const char* path,
                         const char* include_file,
                         struct source_location include_loc) {
    FILE* file = fopen(path, "r");
    if (!file) {
        file_err(state->err, path, include_file, include_loc, true);
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
    fclose(file);

    if (!res) {
        return false;
    }

    return true;
}

static void file_err(struct preproc_err* err,
                     const char* path,
                     const char* include_file,
                     struct source_location include_loc,
                     bool open_fail) {
    assert(path);
    
    char* file = include_file == NULL ? NULL : alloc_string_copy(include_file);
    set_preproc_err(err, PREPROC_ERR_FILE_FAIL, file, include_loc);
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

static void append_terminator_token(struct token** tokens, size_t len) {
    *tokens = xrealloc(*tokens, sizeof(struct token) * (len + 1));
    (*tokens)[len] = (struct token){
        .type = INVALID,
        .spelling = NULL,
        .file = NULL,
        .source_loc = {(size_t)-1, (size_t)-1},
    };
}

static bool preproc_statement(struct preproc_state* state,
                              size_t line_start,
                              struct code_source* src) {
    assert(src != NULL);
    assert(state->tokens[line_start].type == STRINGIFY_OP);
    (void)state;
    (void)line_start;
    (void)src;
    // TODO:
    return false;
}

static inline bool is_spelling(const char* spelling, enum token_type type) {
    const char* expected_spell = get_spelling(type);
    assert(expected_spell != NULL);
    return strcmp(spelling, get_spelling(type)) == 0;
}

static enum token_type keyword_type(const char* spell) {
    if (is_spelling(spell, FUNC_NAME)) {
        return FUNC_NAME;
    } else if (is_spelling(spell, SIZEOF)) {
        return SIZEOF;
    } else if (is_spelling(spell, TYPEDEF)) {
        return TYPEDEF;
    } else if (is_spelling(spell, EXTERN)) {
        return EXTERN;
    } else if (is_spelling(spell, STATIC)) {
        return STATIC;
    } else if (is_spelling(spell, AUTO)) {
        return AUTO;
    } else if (is_spelling(spell, REGISTER)) {
        return REGISTER;
    } else if (is_spelling(spell, INLINE)) {
        return INLINE;
    } else if (is_spelling(spell, BOOL)) {
        return BOOL;
    } else if (is_spelling(spell, CHAR)) {
        return CHAR;
    } else if (is_spelling(spell, SHORT)) {
        return SHORT;
    } else if (is_spelling(spell, INT)) {
        return INT;
    } else if (is_spelling(spell, LONG)) {
        return LONG;
    } else if (is_spelling(spell, SIGNED)) {
        return SIGNED;
    } else if (is_spelling(spell, UNSIGNED)) {
        return UNSIGNED;
    } else if (is_spelling(spell, FLOAT)) {
        return FLOAT;
    } else if (is_spelling(spell, DOUBLE)) {
        return DOUBLE;
    } else if (is_spelling(spell, VOID)) {
        return VOID;
    } else if (is_spelling(spell, COMPLEX)) {
        return COMPLEX;
    } else if (is_spelling(spell, IMAGINARY)) {
        return IMAGINARY;
    } else if (is_spelling(spell, CONST)) {
        return CONST;
    } else if (is_spelling(spell, VOLATILE)) {
        return VOLATILE;
    } else if (is_spelling(spell, RESTRICT)) {
        return RESTRICT;
    } else if (is_spelling(spell, ATOMIC)) {
        return ATOMIC;
    } else if (is_spelling(spell, STRUCT)) {
        return STRUCT;
    } else if (is_spelling(spell, UNION)) {
        return UNION;
    } else if (is_spelling(spell, ENUM)) {
        return ENUM;
    } else if (is_spelling(spell, CASE)) {
        return CASE;
    } else if (is_spelling(spell, DEFAULT)) {
        return DEFAULT;
    } else if (is_spelling(spell, IF)) {
        return IF;
    } else if (is_spelling(spell, ELSE)) {
        return ELSE;
    } else if (is_spelling(spell, SWITCH)) {
        return SWITCH;
    } else if (is_spelling(spell, WHILE)) {
        return WHILE;
    } else if (is_spelling(spell, DO)) {
        return DO;
    } else if (is_spelling(spell, FOR)) {
        return FOR;
    } else if (is_spelling(spell, GOTO)) {
        return GOTO;
    } else if (is_spelling(spell, CONTINUE)) {
        return CONTINUE;
    } else if (is_spelling(spell, BREAK)) {
        return BREAK;
    } else if (is_spelling(spell, RETURN)) {
        return RETURN;
    } else if (is_spelling(spell, ALIGNAS)) {
        return ALIGNAS;
    } else if (is_spelling(spell, ALIGNOF)) {
        return ALIGNOF;
    } else if (is_spelling(spell, GENERIC)) {
        return GENERIC;
    } else if (is_spelling(spell, NORETURN)) {
        return NORETURN;
    } else if (is_spelling(spell, STATIC_ASSERT)) {
        return STATIC_ASSERT;
    } else if (is_spelling(spell, THREAD_LOCAL)) {
        return THREAD_LOCAL;
    } else {
        return IDENTIFIER;
    }
}

