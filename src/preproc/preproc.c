#include "preproc/preproc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util.h"
#include "error.h"

#include "preproc/preproc_state.h"
#include "preproc/tokenizer.h"

static bool preproc_file(struct preproc_state* state, const char* path);
static enum token_type keyword_type(const char* spelling);

void append_terminator_token(struct token** tokens, size_t len);
void convert_preproc_tokens(struct token* tokens);

struct token* preproc(const char* path) {
    struct preproc_state state = {
        .len = 0,
        .alloc_len = 0,
        .tokens = NULL,
    };

    if (!preproc_file(&state, path)) {
        for (size_t i = 0; i < state.len; ++i) {
            free_token(&state.tokens[i]);
        }
        free(state.tokens);
        return NULL;
    }

    append_terminator_token(&state.tokens, state.len);

    convert_preproc_tokens(state.tokens);

    return state.tokens;
}

void append_terminator_token(struct token** tokens, size_t len) {
    *tokens = xrealloc(*tokens, sizeof(struct token) * (len + 1));
    (*tokens)[len] = (struct token){
        .type = INVALID,
        .spelling = NULL,
        .file = NULL,
        .source_loc = {(size_t)-1, (size_t)-1},
    };
}

void convert_preproc_tokens(struct token* tokens) {
    for (struct token* t = tokens; t->type != INVALID; ++t) {
        if (t->type == IDENTIFIER) {
            t->type = keyword_type(t->spelling);
            if (t->type != IDENTIFIER) {
                free(t->spelling);
                t->spelling = NULL;
            }
        }
    }
}

static bool preproc_statement(struct preproc_state* res,
                              const char* line,
                              size_t line_num);

struct token* preproc_string(const char* str, const char* path) {
    struct preproc_state res = {
        .len = 0,
        .alloc_len = 0,
        .tokens = NULL,
    };

    const char* it = str;

    const char* start = str;
    bool comment_not_terminated = false;

    size_t line_num = 1;
    while (true) {
        if (*it == '\n' || *it == '\0') {
            size_t len = it - start + 1;
            char* line = xmalloc(sizeof(char) * len + 1);
            memcpy(line, start, sizeof(char) * len);
            line[len] = '\0';

            if ((line[0] == '#' && !preproc_statement(&res, line, line_num))
                || !tokenize_line(&res,
                                  line,
                                  line_num,
                                  path,
                                  &comment_not_terminated)) {
                free(line);
                goto fail;
            }

            free(line);
            ++line_num;
            start = it + 1;
            if (*it == '\0') {
                break;
            }
        }
        ++it;
    }

    append_terminator_token(&res.tokens, res.len);
    convert_preproc_tokens(res.tokens);

    return res.tokens;

fail:
    for (size_t i = 0; i < res.len; ++i) {
        free_token(&res.tokens[i]);
    }
    free(res.tokens);
    return NULL;
}

void free_tokens(struct token* tokens) {
    for (struct token* it = tokens; it->type != INVALID; ++it) {
        free_token(it);
    }
    free(tokens);
}

static bool preproc_file(struct preproc_state* res, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        // TODO: error
        return false;
    }

    enum { LINE_BUF_LEN = 1000 };
    char line_buf[LINE_BUF_LEN];

    const char LAST_PLACEHOLDER = '$';
    line_buf[LINE_BUF_LEN - 1] = LAST_PLACEHOLDER;
    line_buf[LINE_BUF_LEN - 2] = LAST_PLACEHOLDER;

    bool comment_not_terminated = false;
    size_t line_num = 1;
    // TODO: handle escaped newlines
    // Should be read into the same buffer as the previous line
    while (true) {
        char* ret = fgets(line_buf, LINE_BUF_LEN, file);
        if (ret == NULL) {
            break;
        }

        char* line = line_buf;

        if (line_buf[LINE_BUF_LEN - 1] == '\0'
            && line_buf[LINE_BUF_LEN - 2] != '\n') {
            int len = LINE_BUF_LEN * 2;
            char* dyn_buf = xmalloc(sizeof(char) * len);
            memcpy(dyn_buf, line_buf, LINE_BUF_LEN * sizeof(char));

            dyn_buf[len - 1] = LAST_PLACEHOLDER;
            dyn_buf[len - 2] = LAST_PLACEHOLDER;

            ret = fgets(dyn_buf + LINE_BUF_LEN - 1, len - LINE_BUF_LEN, file);
            while (ret != NULL && dyn_buf[len - 1] == '\0'
                   && dyn_buf[len - 2] != '\n') {
                int prev_len = len;
                len *= 2;
                dyn_buf = xrealloc(dyn_buf, len * sizeof(char));

                dyn_buf[len - 1] = LAST_PLACEHOLDER;
                dyn_buf[len - 2] = LAST_PLACEHOLDER;

                ret = fgets(dyn_buf + len - 1, len - prev_len, file);
            }

            line = dyn_buf;
        }

        if ((line[0] == '#' && !preproc_statement(res, line, line_num))
            || !tokenize_line(res,
                              line,
                              line_num,
                              path,
                              &comment_not_terminated)) {
            goto fail;
        }

        ++line_num;

        if (line != line_buf) {
            free(line);
        }

        line_buf[LINE_BUF_LEN - 1] = LAST_PLACEHOLDER;
        line_buf[LINE_BUF_LEN - 2] = LAST_PLACEHOLDER;
    }

    if (fclose(file) != 0) {
        // TODO: error
        return false;
    }
    return true;

fail:
    if (fclose(file) != 0) {
        // TODO: wat do
    }
    return false;
}

static bool preproc_statement(struct preproc_state* res,
                              const char* line,
                              size_t line_num) {
    assert(line != NULL);
    assert(line[0] == '#');
    (void)res;
    (void)line;
    (void)line_num;
    // TODO:
    return false;
}

static inline bool is_spelling(const char* spelling, enum token_type type) {
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
