#include "preproc/preproc_macro.h"

#include <assert.h>
#include <string.h>

#include "util.h"

static bool expand_func_macro(struct preproc_state* state,
                              struct preproc_macro* macro,
                              size_t macro_idx);

static void expand_non_func_macro(struct preproc_state* state,
                                  struct preproc_macro* macro,
                                  size_t macro_idx);

bool expand_preproc_macro(struct preproc_state* state,
                          struct preproc_macro* macro,
                          size_t macro_idx) {
    assert(state);
    assert(macro);
    assert(strcmp(state->tokens[macro_idx].spelling, macro->spelling) == 0);

    if (macro->is_func_macro) {
        return expand_func_macro(state, macro, macro_idx);
    } else {
        expand_non_func_macro(state, macro, macro_idx);
        return true;
    }
}

static struct token copy_token(const struct token* t);

static bool expand_func_macro(struct preproc_state* state,
                              struct preproc_macro* macro,
                              size_t macro_idx) {
    assert(macro->is_func_macro);

    (void)state;
    (void)macro;
    (void)macro_idx;
    // TODO: implement
    return false;
}

static void expand_non_func_macro(struct preproc_state* state,
                                  struct preproc_macro* macro,
                                  size_t macro_idx) {
    assert(macro->is_func_macro == false);
    assert(macro->num_args == 0);

    const size_t exp_len = macro->expansion_len;
    const size_t old_len = state->len;
    state->len += exp_len - 1;
    if (state->cap < state->len) {
        state->tokens = xrealloc(state->tokens,
                                 state->len * sizeof(struct token));
        state->cap = state->len;
    }

    // shift tokens behind macro forward
    for (size_t i = old_len - 1; i > macro_idx; --i) {
        state->tokens[i + exp_len - 1] = state->tokens[i];
    }

    free_token(&state->tokens[macro_idx]);

    for (size_t i = 0; i < exp_len; ++i) {
        const struct token_or_num* curr = &macro->expansion[i];
        assert(!curr->is_arg_num);

        state->tokens[macro_idx + i] = copy_token(&curr->token);
    }
}

static struct token copy_token(const struct token* t) {
    assert(t);
    return (struct token) {
        .type = t->type,
        .spelling = t->spelling == NULL ? NULL : alloc_string_copy(t->spelling),
        .file = alloc_string_copy(t->file),
        .source_loc =
            {
                // TODO: what do we do here?
                .line = t->source_loc.line,
                .index = t->source_loc.index,
            },
    };
}
