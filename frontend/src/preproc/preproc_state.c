#include "frontend/preproc/preproc_state.h"

#include <string.h>

#include "util/mem.h"

#include "frontend/preproc/preproc_macro.h"

struct preproc_state create_preproc_state(const char* start_file, struct preproc_err* err) {
    struct str file_name = create_str(strlen(start_file), start_file);
    FILE* file = fopen(start_file, "r");
    if (!file) {
        set_preproc_file_err(err, 0, (struct source_loc){0, {0, 0}}, true);
        return (struct preproc_state){0};
    }
    return (struct preproc_state){
        .res =
            {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            },
        .line_info = {
            .line = create_empty_str(),
            .next = NULL,
            .curr_loc = {
                .file_idx = 0,
                .file_loc = {0, 1},
            },
        },
        .file = file,
        .conds_len = 0,
        .conds_cap = 0,
        .conds = NULL,
        .err = err,
        ._macro_map = create_string_hash_map(
            sizeof(struct preproc_macro),
            100,
            true,
            (void (*)(void*))free_preproc_macro),
        .file_info = create_file_info(&file_name),
    };
}

struct preproc_state create_preproc_state_string(const char* code, const char* filename, struct preproc_err* err) {
    struct str filename_str = create_str(strlen(filename), filename);
    return (struct preproc_state){
        .res = {
            .len = 0,
            .cap = 0,
            .tokens = NULL,
        },
        .line_info = {
            .line = create_empty_str(),
            .next = code,
            .curr_loc = {
                .file_idx = 0,
                .file_loc = {1, 1},
            },
        },
        .file = NULL,
        .conds_len = 0,
        .conds_cap = 0,
        .conds = NULL,
        .err = err,
        ._macro_map = create_string_hash_map(sizeof(struct preproc_macro), 100, true, (void (*)(void*))free_preproc_macro),
        .file_info = create_file_info(&filename_str),
    };
}

void preproc_state_read_line(struct preproc_state* state) {
    assert(state);
    str_clear(&state->line_info.line); 
    int c;
    while ((c = getc(state->file)) != '\n' && c != '\r' && c != EOF) {
        str_push_back(&state->line_info.line, (char)c);
    }

#ifndef _WIN32
    if (c == '\r') {
        int next = getc(state->file);
        if (next != '\n') {
            putc(next, state->file);
        }
    }
#endif
    state->line_info.next = str_get_data(&state->line_info.line);
    state->line_info.curr_loc.file_loc.line += 1;
    state->line_info.curr_loc.file_loc.index = 1;
}

bool preproc_state_over(const struct preproc_state* state) {
    return (state->line_info.next == NULL || *state->line_info.next == '\0') && (state->file == NULL || feof(state->file));
}

const struct preproc_macro* find_preproc_macro(const struct preproc_state* state,
                                               const struct str* spelling) {
    return string_hash_map_get(&state->_macro_map, spelling);
}

void register_preproc_macro(struct preproc_state* state,
                            const struct str* spelling,
                            const struct preproc_macro* macro) {
    bool overwritten = string_hash_map_insert_overwrite(&state->_macro_map, spelling, macro);
    (void)overwritten; // TODO: warning if redefined
}

void remove_preproc_macro(struct preproc_state* state, const struct str* spelling) {
    string_hash_map_remove(&state->_macro_map, spelling);
}

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true) {
    if (state->conds_len == state->conds_cap) {
        mycc_grow_alloc((void**)&state->conds,
                   &state->conds_cap,
                   sizeof *state->conds);
    }

    struct preproc_cond c = {
        .had_true_branch = was_true,
        .had_else = false,
        .loc = loc,
    };
    state->conds[state->conds_len] = c;
    ++state->conds_len;
}

void pop_preproc_cond(struct preproc_state* state) {
    --state->conds_len;
}

struct preproc_cond* peek_preproc_cond(struct preproc_state* state) {
    return &state->conds[state->conds_len - 1];
}

void free_token_arr(struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        free_str(&arr->tokens[i].spelling);
    }
    mycc_free(arr->tokens);
}

static void free_line_info(struct line_info* info) {
    free_str(&info->line);
}

void free_preproc_state(struct preproc_state* state) {
    free_token_arr(&state->res);
    free_line_info(&state->line_info);
    if (state->file) {
        fclose(state->file);
    }
    mycc_free(state->conds);
    free_string_hash_map(&state->_macro_map);
    free_file_info(&state->file_info);
}

