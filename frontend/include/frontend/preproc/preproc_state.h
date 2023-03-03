#ifndef PREPROC_STATE_H
#define PREPROC_STATE_H

#include <stddef.h>

#include "util/string_hash_map.h"

#include "frontend/file_info.h"

#include "preproc_err.h"

struct token_arr {
    size_t len, cap;
    struct token* tokens;
};

struct line_info {
    struct str line;
    const char* next;
    struct source_loc curr_loc;
    bool is_in_comment;
};

struct preproc_cond {
    bool had_true_branch;
    bool had_else;
    struct source_loc loc;
};

struct preproc_state {
    struct token_arr res;

    struct line_info line_info;
    FILE* file;

    size_t conds_len, conds_cap;
    struct preproc_cond* conds;

    struct preproc_err* err;
    struct string_hash_map _macro_map;
    struct file_info file_info;
};

struct preproc_state create_preproc_state(const char* start_file,
                                          struct preproc_err* err);

struct preproc_state create_preproc_state_string(const char* code, const char* filename, struct preproc_err* err);

void preproc_state_read_line(struct preproc_state* state);
bool preproc_state_over(const struct preproc_state* state);

struct preproc_macro;

const struct preproc_macro* find_preproc_macro(const struct preproc_state* state,
                                               const struct str* spelling);

void register_preproc_macro(struct preproc_state* state,
                            const struct str* spelling,
                            const struct preproc_macro* macro);

void remove_preproc_macro(struct preproc_state* state, const struct str* spelling);

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true);

void pop_preproc_cond(struct preproc_state* state);

struct preproc_cond* peek_preproc_cond(struct preproc_state* state);

void free_token_arr(struct token_arr* arr);

void free_preproc_state(struct preproc_state* state);

#include "preproc_macro.h"

#endif

