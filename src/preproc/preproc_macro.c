#include "preproc/preproc_macro.h"

#include <assert.h>
#include <string.h>

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
        assert(macro_end);
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

static struct token_arr collect_until(struct token* start, const struct token* end) {
    const size_t len = end - start;
    struct token_arr res = {
        .len = len,
        .tokens = len == 0 ? NULL : xmalloc(sizeof(struct token) * len),
    };
    
    for (size_t i = 0; i < len; ++i) {
        res.tokens[i] = move_token(&start[i]);
    }
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

    return collect_until(arg_start, it);
}

struct macro_args {
    size_t len;
    struct token_arr* arrs;
};


static void free_macro_args(struct macro_args* args) {
    for (size_t i = 0; i < args->len; ++i) {
        free_token_arr(&args->arrs[i]);
    }
}

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
                                     size_t expected_args,
                                     bool is_variadic,
                                     struct preproc_err* err) {
    assert(args_start->type == LBRACKET);

    size_t cap = is_variadic ? expected_args + 1 : expected_args;
    struct macro_args res = {
        .len = 0,
        .arrs = xmalloc(sizeof(struct token_arr) * cap),
    };

    struct token* it = args_start + 1;
    while (res.len != expected_args && it != limit_ptr) {
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
    
    if (res.len < expected_args) {
        char* file = it->file;
        it->file = NULL;
        set_preproc_err(err, PREPROC_ERR_MACRO_ARG_COUNT, file, it->source_loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = true;
        goto fail;
    } else if (is_variadic) {
        assert(it->type == COMMA);
        res.arrs[res.len] = collect_until(it + 1, limit_ptr);
        ++res.len;
    } else if (it != limit_ptr) {
        char* file = it->file;
        it->file = NULL;
        set_preproc_err(err, PREPROC_ERR_MACRO_ARG_COUNT, file, it->source_loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = false;
        goto fail;
    }

    return res;
fail:
    free_macro_args(&res);
    return (struct macro_args){
        .len = 0,
        .arrs = NULL,
    };
}

static size_t get_expansion_len(const struct preproc_macro* macro,
                                const struct macro_args* args) {
    size_t len = 0;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const struct token_or_arg* item = &macro->expansion[i];
        if (item->is_arg) {
            len += args->arrs[item->arg_num].len;
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
                                                macro->num_args,
                                                macro->is_variadic,
                                                state->err);
    if (args.len == 0 && macro->num_args != 0) {
        assert(state->err->type != PREPROC_ERR_NONE);
        return false;
    }
    
    assert((macro->is_variadic && args.len == macro->num_args + 1) ||
           (!macro->is_variadic && args.len == macro->num_args));

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
            copy_into_tokens(state->tokens, &token_idx, &args.arrs[curr->arg_num]);
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
