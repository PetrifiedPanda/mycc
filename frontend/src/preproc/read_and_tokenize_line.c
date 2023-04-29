#include "frontend/preproc/read_and_tokenize_line.h"

#include <string.h>
#include <ctype.h>

#include "util/macro_util.h"
#include "util/mem.h"

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
                              struct token_arr* arr);

bool read_and_tokenize_line(struct preproc_state* state) {
    assert(state);

    while (true) {
        if (state->line_info.next == NULL || *state->line_info.next == '\0') {
            preproc_state_read_line(state);
        }
        if (preproc_state_over(state)) {
            return true;
        }

        if (is_preproc_directive(state->line_info.next)) {
            struct token_arr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };

            const bool res = tokenize_line(&arr, state->err, &state->line_info);
            if (!res) {
                free_token_arr(&arr);
                return false;
            }

            const bool stat_res = preproc_statement(state, &arr);
            free_token_arr(&arr);
            if (!stat_res) {
                return false;
            }
        } else {
            const bool res = tokenize_line(&state->res,
                                           state->err,
                                           &state->line_info);
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

    static const char else_dir[] = "else";
    static const char elif_dir[] = "elif";
    static const char endif_dir[] = "endif";

    if (*it == '\0') {
        return false;
    } else if (strncmp(it, else_dir, sizeof else_dir) == 0
               || strncmp(it, elif_dir, sizeof elif_dir) == 0
               || strncmp(it, endif_dir, sizeof endif_dir) == 0) {
        return true;
    } else {
        return false;
    }
}

// TODO: could probably be optimized
static bool skip_until_next_cond(struct preproc_state* state) {
    while (!preproc_state_over(state)) {
        preproc_state_read_line(state);
        if (is_cond_directive(state->line_info.next)) {
            struct token_arr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };
            const bool res = tokenize_line(&arr, state->err, &state->line_info);
            if (!res) {
                return false;
            }

            const bool stat_res = preproc_statement(state, &arr);
            free_token_arr(&arr);
            return stat_res;
        }
    }

    set_preproc_err(state->err,
                    PREPROC_ERR_UNTERMINATED_COND,
                    state->line_info.curr_loc);
    state->err->unterminated_cond_loc = state->conds[state->conds_len - 1].loc;
    return false;
}

static bool handle_preproc_if(struct preproc_state* state,
                              bool cond,
                              struct source_loc loc) {
    push_preproc_cond(state, loc, cond);

    if (!cond) {
        return skip_until_next_cond(state);
    }

    return true;
}

static bool handle_ifdef_ifndef(struct preproc_state* state,
                                struct token_arr* arr,
                                bool is_ifndef) {
    assert(arr);
    assert(arr->tokens[0].kind == TOKEN_PP_STRINGIFY);
    assert(
        (!is_ifndef
         && strcmp(str_get_data(&arr->tokens[1].spelling), "ifdef") == 0)
        || (is_ifndef
            && strcmp(str_get_data(&arr->tokens[1].spelling), "ifndef") == 0));
    const struct source_loc loc = arr->tokens[0].loc;

    if (arr->len < 3) {
        set_preproc_err(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[1].loc);
        state->err->count_empty = true;
        state->err->count_dir_kind = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    } else if (arr->len > 3) {
        set_preproc_err(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[3].loc);
        state->err->count_empty = false;
        state->err->count_dir_kind = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    }
    if (arr->tokens[2].kind != TOKEN_IDENTIFIER) {
        set_preproc_err(state->err,
                        PREPROC_ERR_IFDEF_NOT_ID,
                        arr->tokens[2].loc);
        state->err->not_identifier_got = arr->tokens[2].kind;
        state->err->not_identifier_op = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                                  : SINGLE_MACRO_OP_IFDEF;
        return false;
    }

    const struct str* macro_spell = &arr->tokens[2].spelling;
    assert(macro_spell);
    assert(str_is_valid(macro_spell));
    const struct preproc_macro* macro = find_preproc_macro(state, macro_spell);

    const bool cond = is_ifndef ? macro == NULL : macro != NULL;
    return handle_preproc_if(state, cond, loc);
}

static bool handle_else_elif(struct preproc_state* state,
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
        return skip_until_next_cond(state);
    } else if (is_else) {
        curr_if->had_else = true;
        // TODO: just continue
    } else {
        // TODO: evaluate condition
        (void)arr;
    }

    return false;
}

static bool handle_include(struct preproc_state* state, struct token_arr* arr) {
    assert(arr->len >= 2);
    if (arr->len == 2 || arr->len > 3) {
        set_preproc_err(state->err,
                        PREPROC_ERR_INCLUDE_NUM_ARGS,
                        arr->tokens[1].loc);
        return false;
    }

    if (arr->tokens[2].kind != TOKEN_STRING_LITERAL) {
        if (!expand_all_macros(state, arr, 2)) {
            return false;
        }
    }

    // TODO: "<" ">" string literals
    if (arr->tokens[2].kind == TOKEN_STRING_LITERAL) {
        struct str_lit filename = convert_to_str_lit(&arr->tokens[2].spelling);
        if (filename.kind != STR_LIT_DEFAULT) {
            set_preproc_err(state->err,
                            PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
                            arr->tokens[2].loc);
            return false;
        }
        if (!preproc_state_open_file(state,
                                     &filename.contents,
                                     arr->tokens[2].loc)) {
            return false;
        }
        return true;
    } else {
        set_preproc_err(state->err,
                        PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
                        arr->tokens[2].loc);
        return false;
    }
}

static bool preproc_statement(struct preproc_state* state,
                              struct token_arr* arr) {
    assert(arr);
    assert(arr->tokens);
    assert(arr->tokens[0].kind == TOKEN_PP_STRINGIFY);
    if (arr->len == 1) {
        return true;
    } else if (arr->tokens[1].kind != TOKEN_IDENTIFIER) {
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
        return handle_ifdef_ifndef(state, arr, false);
    } else if (strcmp(directive, "ifndef") == 0) {
        return handle_ifdef_ifndef(state, arr, true);
    } else if (strcmp(directive, "define") == 0) {
        const struct str spell = token_take_spelling(&arr->tokens[2]);
        struct preproc_macro macro = parse_preproc_macro(arr, state->err);
        if (state->err->kind != PREPROC_ERR_NONE) {
            return false;
        }
        register_preproc_macro(state, &spell, &macro);
    } else if (strcmp(directive, "undef") == 0) {
        if (arr->len < 3) {
            set_preproc_err(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[1].loc);
            state->err->count_empty = true;
            state->err->count_dir_kind = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->len > 3) {
            set_preproc_err(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[3].loc);
            state->err->count_empty = false;
            state->err->count_dir_kind = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->tokens[2].kind != TOKEN_IDENTIFIER) {
            set_preproc_err(state->err,
                            PREPROC_ERR_IFDEF_NOT_ID,
                            arr->tokens[2].loc);
            state->err->not_identifier_got = arr->tokens[2].kind;
            state->err->not_identifier_op = SINGLE_MACRO_OP_UNDEF;
            return false;
        }

        remove_preproc_macro(state, &arr->tokens[2].spelling);
    } else if (strcmp(directive, "include") == 0) {
        return handle_include(state, arr);
    } else if (strcmp(directive, "pragma") == 0) {
        // TODO:
    } else if (strcmp(directive, "elif") == 0) {
        return handle_else_elif(state, arr, false);
    } else if (strcmp(directive, "else") == 0) {
        return handle_else_elif(state, arr, true);
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

