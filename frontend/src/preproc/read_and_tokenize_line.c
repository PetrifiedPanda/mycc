#include "frontend/preproc/read_and_tokenize_line.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util/macro_util.h"

#include "frontend/preproc/tokenizer.h"

enum {
    PREPROC_LINE_BUF_LEN = 200
};

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
bool read_and_tokenize_line(struct preproc_state* state,
                            struct code_source* src) {
    assert(src);

    while (true) {
        char static_buf[PREPROC_LINE_BUF_LEN];
        const size_t prev_curr_line = src->current_line;
        char* line = code_source_read_line(src,
                                           PREPROC_LINE_BUF_LEN,
                                           static_buf);
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
                                           state->file_info.len - 1,
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
                                     state->file_info.len - 1,
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

static bool is_cond_directive(const char* line) {
    const char* it = line;
    while (*it != '\0' && isspace(*it)) {
        ++it;
    }

    if (*it != '#') {
        return false;
    }

    ++it;
    while (*it != '\0' && isspace(*it)) {
        ++it;
    }

    const char else_dir[] = "else";
    const char elif_dir[] = "elif";
    const char endif_dir[] = "endif";

    if (*it == '\0') {
        return false;
    } else if (strncmp(it, else_dir, sizeof else_dir) == 0
               || strncmp(it, elif_dir, sizeof elif_dir) == 0
               || strncmp(it, endif_dir, sizeof endif_dir)) {
        return true;
    }

    UNREACHABLE();
}

// TODO: could probably be optimized
static bool skip_until_next_cond(struct preproc_state* state,
                                 struct code_source* src) {
    while (!code_source_over(src)) {
        char static_buf[PREPROC_LINE_BUF_LEN];
        const size_t prev_curr_line = src->current_line;
        char* line = code_source_read_line(src,
                                           PREPROC_LINE_BUF_LEN,
                                           static_buf);
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
                                           state->file_info.len - 1,
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
        .file_idx = state->file_info.len - 1,
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
    assert(
        (!is_ifndef
         && strcmp(str_get_data(&arr->tokens[1].spelling), "ifdef") == 0)
        || (is_ifndef
            && strcmp(str_get_data(&arr->tokens[1].spelling), "ifndef") == 0));

    if (arr->len < 3) {
        set_preproc_err(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[1].loc);
        state->err->count_empty = true;
        state->err->count_dir_type = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    } else if (arr->len > 3) {
        set_preproc_err(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[3].loc);
        state->err->count_empty = false;
        state->err->count_dir_type = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    }
    if (arr->tokens[2].type != IDENTIFIER) {
        set_preproc_err(state->err,
                        PREPROC_ERR_IFDEF_NOT_ID,
                        arr->tokens[2].loc);
        state->err->not_identifier_got = arr->tokens[2].type;
        state->err->not_identifier_op = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                                  : SINGLE_MACRO_OP_IFDEF;
        return false;
    }

    const struct str* macro_spell = &arr->tokens[2].spelling;
    assert(macro_spell);
    assert(str_is_valid(macro_spell));
    const struct preproc_macro* macro = find_preproc_macro(state, macro_spell);

    const bool cond = is_ifndef ? macro == NULL : macro != NULL;
    return handle_preproc_if(state, src, cond);
}

static bool handle_else_elif(struct preproc_state* state,
                             struct code_source* src,
                             struct token_arr* arr,
                             bool is_else) {
    assert(arr->len > 1);
    if (state->conds_len == 0) {
        set_preproc_err(state->err, PREPROC_ERR_MISSING_IF, arr->tokens[1].loc);
        state->err->missing_if_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        return false;
    }

    struct preproc_cond* curr_if = peek_preproc_cond(state);
    if (curr_if->had_else) {
        set_preproc_err(state->err,
                        PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
                        arr->tokens[1].loc);
        state->err->elif_after_else_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        state->err->prev_else_loc = curr_if->loc;
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
    if (arr->len == 1) {
        return true;
    } else if (arr->tokens[1].type != IDENTIFIER) {
        set_preproc_err(state->err,
                        PREPROC_ERR_INVALID_PREPROC_DIR,
                        arr->tokens[1].loc);
        return false;
    }

    const char* directive = str_get_data(&arr->tokens[1].spelling);
    assert(directive);

    if (strcmp(directive, "if") == 0) {
        // TODO:
    } else if (strcmp(directive, "ifdef") == 0) {
        return handle_ifdef_ifndef(state, src, arr, false);
    } else if (strcmp(directive, "ifndef") == 0) {
        return handle_ifdef_ifndef(state, src, arr, true);
    } else if (strcmp(directive, "define") == 0) {
        const struct str spell = take_spelling(&arr->tokens[2]);
        struct preproc_macro macro = parse_preproc_macro(arr,
                                                         str_get_data(&spell),
                                                         state->err);
        if (state->err->type != PREPROC_ERR_NONE) {
            return false;
        }
        register_preproc_macro(state, &spell, &macro);
    } else if (strcmp(directive, "undef") == 0) {
        if (arr->len < 3) {
            set_preproc_err(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[1].loc);
            state->err->count_empty = true;
            state->err->count_dir_type = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->len > 3) {
            set_preproc_err(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[3].loc);
            state->err->count_empty = false;
            state->err->count_dir_type = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->tokens[2].type != IDENTIFIER) {
            set_preproc_err(state->err,
                            PREPROC_ERR_IFDEF_NOT_ID,
                            arr->tokens[2].loc);
            state->err->not_identifier_got = arr->tokens[2].type;
            state->err->not_identifier_op = SINGLE_MACRO_OP_UNDEF;
            return false;
        }

        remove_preproc_macro(state, &arr->tokens[2].spelling);
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
            set_preproc_err(state->err,
                            PREPROC_ERR_MISSING_IF,
                            arr->tokens[1].loc);
            state->err->missing_if_op = ELSE_OP_ENDIF;
            return false;
        }

        pop_preproc_cond(state);
    } else {
        set_preproc_err(state->err,
                        PREPROC_ERR_INVALID_PREPROC_DIR,
                        arr->tokens[1].loc);
        return false;
    }

    return true;
}
