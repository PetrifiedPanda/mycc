#include "preproc/preproc_state.h"

#include <stdlib.h>

#include "util/mem.h"

struct preproc_macro* find_preproc_macro(struct preproc_state* state,
                                         const char* spelling) {
    // TODO: implement
    (void)state;
    (void)spelling;
    return NULL;
}

void register_preproc_macro(struct preproc_state* state,
                            const struct preproc_macro* macro) {
    // TODO: implement
    (void)state;
    (void)macro;
}

void remove_preproc_macro(struct preproc_state* state, const char* spelling) {
    // TODO: implement
    (void)state;
    (void)spelling;
}

void push_preproc_cond(struct preproc_state* state,
                       struct source_loc loc,
                       bool was_true) {
    if (state->conds_len == state->conds_cap) {
        grow_alloc((void**)&state->conds,
                   &state->conds_cap,
                   sizeof(struct preproc_cond));
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
    free_source_loc(&state->conds[state->conds_len - 1].loc);
    --state->conds_len;
}

struct preproc_cond* peek_preproc_cond(struct preproc_state* state) {
    return &state->conds[state->conds_len - 1];
}

void free_token_arr(struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        free_token(&arr->tokens[i]);
    }
    free(arr->tokens);
}

void free_preproc_state(struct preproc_state* state) {
    free(state->conds);
}

