#include "preproc/preproc_macro.h"

#include <assert.h>
#include <string.h>

#include "error.h"

#include "util/mem.h"

static bool expand_func_macro(struct preproc_state* state,
                              const struct preproc_macro* macro,
                              size_t macro_idx,
                              const struct token* macro_end);

static void expand_obj_macro(struct preproc_state* state,
                             const struct preproc_macro* macro,
                             size_t macro_idx);

bool expand_preproc_macro(struct preproc_state* state,
                          const struct preproc_macro* macro,
                          size_t macro_idx,
                          const struct token* macro_end) {
    assert(state);
    assert(macro);
    assert(strcmp(state->tokens[macro_idx].spelling, macro->spelling) == 0);

    if (macro->is_func_macro) {
        assert(macro_end->type == RBRACKET);
        return expand_func_macro(state, macro, macro_idx, macro_end);
    } else {
        expand_obj_macro(state, macro, macro_idx);
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

static struct token move_token(struct token* t) {
    struct token res = *t;
    t->spelling = NULL;
    t->file = NULL;
    return res;
}

/**
 *
 * @param it Token pointer to the start of a macro argument
 * @param limit_ptr Pointer to this macros closing bracket
 *
 * @return The macro argument given at the start of this pointer
 */
static struct token_arr collect_macro_arg(struct token* it,
                                          const struct token* limit_ptr) {
    size_t num_open_brackets = 0;
    struct token* arg_start = it;
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
        res.tokens[i] = move_token(&arg_start[i]);
    }

    return res;
}

struct macro_args {
    struct token_arr* arrs;
    size_t len;
};

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
struct macro_args collect_macro_args(struct token* args_start,
                                     const struct token* limit_ptr,
                                     size_t expected_args) {
    assert(args_start->type == LBRACKET);

    size_t cap = expected_args;
    struct macro_args res = {
        .len = 0,
        .arrs = xmalloc(sizeof(struct token_arr) * cap),
    };

    struct token* it = args_start + 1;
    while (it != limit_ptr) {
        if (res.len == cap) {
            grow_alloc((void**)&res.arrs, &cap, sizeof(struct token_arr));
        }

        res.arrs[res.len] = collect_macro_arg(it, limit_ptr);
        it += res.arrs[res.len].len;

        if (it->type == COMMA) {
            ++it;
        }
        ++res.len;
    }

    assert(it->type == RBRACKET);

    return res;
}

static void free_macro_args(struct macro_args* args) {
    for (size_t i = 0; i < args->len; ++i) {
        free_token_arr(&args->arrs[i]);
    }
}

static size_t get_va_args_len(const struct preproc_macro* macro,
                              const struct macro_args* args) {
    size_t va_args_len = 0;
    for (size_t i = macro->num_args; i < args->len; ++i) {
        va_args_len += args->arrs[i].len;
    }
    return va_args_len;
}

static size_t get_expansion_len(const struct preproc_macro* macro,
                                const struct macro_args* args) {
    const size_t va_args_len = get_va_args_len(macro, args);

    size_t len = 0;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        struct token_or_arg* item = &macro->expansion[i];
        if (item->is_arg) {
            if (item->is_va_args) {
                len += va_args_len;
            } else {
                len += args->arrs[item->arg_num].len;
            }
        } else {
            len += 1;
        }
    }

    return len;
}

static void shift_back(struct token* tokens, size_t num, size_t from, size_t to) {
    for (size_t i = to - 1; i > from; --i) {
        tokens[i + num - 1] = tokens[i];
    }
}

static void shift_forward(struct token* tokens, size_t num, size_t from, size_t to) {
    for (size_t i = from; i < to; ++i) {
        tokens[i] = tokens[i + num];
    }
}

static void copy_into_tokens(struct token* tokens, size_t* token_idx, const struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        tokens[*token_idx] = copy_token(&arr->tokens[i]);
        ++*token_idx;
    }
}

// TODO: stringification and concatenation and definitely some memory leaks
static bool expand_func_macro(struct preproc_state* state,
                              const struct preproc_macro* macro,
                              size_t macro_idx,
                              const struct token* macro_end) {
    assert(macro->is_func_macro);
    assert(state->tokens[macro_idx + 1].type == LBRACKET);
    struct macro_args args = collect_macro_args(state->tokens + macro_idx + 1,
                                                macro_end,
                                                macro->num_args);
    if (args.len < macro->num_args) {
        const struct token* err_token = &state->tokens[macro_idx];

        const char* at_least_str = macro->is_variadic ? " at least" : "";
        set_error_file(ERR_PREPROC,
                       err_token->file, 
                       err_token->source_loc, 
                       "Too few arguments for function-like macro %s: " 
                       "Expected%s %zu arguments but got %zu",
                       err_token->spelling,
                       at_least_str,
                       macro->num_args,
                       args.len);
        return false;
    }

    const size_t exp_len = get_expansion_len(macro, &args);
    const size_t macro_call_len = macro_end - &state->tokens[macro_idx];
    
    const bool alloc_grows = exp_len > macro_call_len;
    const size_t alloc_change = alloc_grows
                                    ? exp_len - macro_call_len
                                    : macro_call_len - exp_len;
    
    const size_t old_len = state->len;
    if (alloc_grows) { 
        state->len += alloc_change;
        state->cap += alloc_change;
        state->tokens = xrealloc(state->tokens,
                                 sizeof(struct token) * state->cap);
 
        shift_back(state->tokens, alloc_change, macro_idx, old_len);         
    } else if (alloc_change != 0) {
        state->len -= alloc_change;

        shift_forward(state->tokens, alloc_change, macro_idx, old_len);
    }

    size_t token_idx = macro_idx;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const struct token_or_arg* curr = &macro->expansion[i];
        assert(state->tokens + token_idx < macro_end);
        if (curr->is_arg) {
            if (curr->is_va_args) {
                assert(macro->is_variadic);
                for (size_t j = macro->num_args; j < args.len; ++j) {
                    copy_into_tokens(state->tokens, &token_idx, &args.arrs[j]);
                    if (i != args.len - 1) {
                        // TODO: filename and location
                        state->tokens[token_idx] = create_token(COMMA, NULL, (struct source_location){0, 0}, "AAAAH");
                        ++token_idx;
                    }
                }

            } else {
                copy_into_tokens(state->tokens, &token_idx, &args.arrs[curr->arg_num]);
            }
        } else {
            state->tokens[token_idx] = copy_token(&curr->token);
            ++token_idx;
        }
    }
 
    free_macro_args(&args);
    return true;
}

static void expand_obj_macro(struct preproc_state* state,
                             const struct preproc_macro* macro,
                             size_t macro_idx) {
    assert(macro->is_func_macro == false);
    assert(macro->num_args == 0);

    const size_t exp_len = macro->expansion_len;
    const size_t old_len = state->len;

    free_token(&state->tokens[macro_idx]);

    if (exp_len > 0) {
        state->cap += exp_len - 1;
        state->len += exp_len - 1;
        state->tokens = xrealloc(state->tokens,
                                 sizeof(struct token) * state->cap);
        
        shift_back(state->tokens, exp_len, macro_idx, old_len);
    } else {
        state->len -= 1;
        
        shift_forward(state->tokens, 1, macro_idx, old_len);
    }

    for (size_t i = 0; i < exp_len; ++i) {
        const struct token_or_arg* curr = &macro->expansion[i];
        assert(!curr->is_arg);

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
                // TODO: identify as token from macro expansion
                .line = t->source_loc.line,
                .index = t->source_loc.index,
            },
    };
}
