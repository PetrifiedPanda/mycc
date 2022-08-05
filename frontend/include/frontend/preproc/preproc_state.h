#ifndef PREPROC_STATE_H
#define PREPROC_STATE_H

#include <stddef.h>

#include "util/string_hash_map.h"

#include "preproc_err.h"

struct token_arr {
    size_t len, cap;
    struct token* tokens;
};

struct preproc_cond {
    bool had_true_branch;
    bool had_else;
    struct source_loc loc;
};

struct preproc_state {
    struct token_arr res;
    size_t conds_len, conds_cap;
    struct preproc_cond* conds;
    struct preproc_err* err;
    struct string_hash_map _macro_map;
};

struct preproc_state create_preproc_state(struct preproc_err* err);

struct preproc_macro;

const struct preproc_macro* find_preproc_macro(struct preproc_state* state,
                                               const char* spelling);

void register_preproc_macro(struct preproc_state* state,
                            char* spelling,
                            const struct preproc_macro* macro);

void remove_preproc_macro(struct preproc_state* state, const char* spelling);

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true);

void pop_preproc_cond(struct preproc_state* state);

struct preproc_cond* peek_preproc_cond(struct preproc_state* state);

void free_token_arr(struct token_arr* arr);

void free_preproc_state(struct preproc_state* state);

#include "preproc_macro.h"

#endif

