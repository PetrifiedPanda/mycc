#include "preproc/preproc_macro.h"

#include <assert.h>
#include <string.h>

#include "util.h"

static bool expand_func_macro(struct preproc_state* state,
                              struct preproc_macro* macro,
                              size_t macro_idx,
                              struct token* macro_end);

static void expand_non_func_macro(struct preproc_state* state,
                                  struct preproc_macro* macro,
                                  size_t macro_idx);

bool expand_preproc_macro(struct preproc_state* state,
                          struct preproc_macro* macro,
                          size_t macro_idx,
                          struct token* macro_end) {
    assert(state);
    assert(macro);
    assert(strcmp(state->tokens[macro_idx].spelling, macro->spelling) == 0);

    if (macro->is_func_macro) {
        assert(macro_end->type == RBRACKET);
        return expand_func_macro(state, macro, macro_idx, macro_end);
    } else {
        expand_non_func_macro(state, macro, macro_idx);
        return true;
    }
}

static struct token copy_token(const struct token* t);

struct token_arr {
    size_t len;
    struct token* tokens;
};

static void free_token_arr(struct token_arr* arr) {
    assert(arr);
    for (size_t i = 0; i < arr->len; ++i) {
        free_token(&arr->tokens[i]);
    }
}

/**
 *
 * @param it Token pointer to the start of a macro argument
 * @param limit_ptr Pointer to this macros closing bracket
 *
 * @return The macro argument given at the start of this pointer
 */
static struct token_arr collect_macro_arg(const struct token* it,
                                          const struct token* limit_ptr) {
    size_t num_open_brackets = 0;
    const struct token* arg_start = it;
    while (it != limit_ptr && it->type != COMMA
           && (it->type != RBRACKET || num_open_brackets != 0)) {
        if (it->type == LBRACKET) {
            ++num_open_brackets;
        } else if (it->type == RBRACKET) {
            --num_open_brackets;
        }

        ++it;
    }

    const size_t arg_len = it - arg_start;
    struct token_arr res = {
        .len = arg_len,
        .tokens = arg_len == 0 ? NULL : xmalloc(sizeof(struct token) * arg_len),
    };

    for (size_t i = 0; i < arg_len; ++i) {
        res.tokens[i] = copy_token(&arg_start[i]);
    }

    return res;
}

// TODO: maybe it is worth it to move the tokens for the parameters
/**
 * @brief Collects arguments for the given macro, assuming the number of
 *        arguments was already checked in the course of finding the closing
 *        bracket
 *
 * @param args_start Pointer to start of macro arguments (opening bracket)
 * @param limit_ptr Pointer to closing bracket of this macro
 * @param expected_args Number of arguments this macro expects
 *
 * @return Pointer to #expected_args token_arrs representing the arguments of
 *         this macro call
 */
struct token_arr* collect_macro_args(const struct token* args_start,
                                     const struct token* limit_ptr,
                                     size_t expected_args) {
    assert(args_start->type == LBRACKET);

    struct token_arr* args = xmalloc(sizeof(struct token_arr) * expected_args);

    const struct token* it = args_start + 1;
    size_t arg_num = 0;
    while (it != limit_ptr) {
        assert(arg_num < expected_args);

        args[arg_num] = collect_macro_arg(it, limit_ptr);
        it += args[arg_num].len;

        if (it->type == COMMA) {
            ++it;
        }
    }

    assert(it->type == RBRACKET);

    return args;
}

static bool expand_func_macro(struct preproc_state* state,
                              struct preproc_macro* macro,
                              size_t macro_idx,
                              struct token* macro_end) {
    assert(macro->is_func_macro);
    assert(state->tokens[macro_idx + 1].type == LBRACKET);

    (void)state;
    (void)macro;
    (void)macro_idx;
    (void)macro_end;
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
    return (struct token){
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
