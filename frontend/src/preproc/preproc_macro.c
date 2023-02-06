#include "frontend/preproc/preproc_macro.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "frontend/token.h"
#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/preproc/read_and_tokenize_line.h"

// TODO: make this also collect macro args?
static size_t find_macro_end(struct preproc_state* state,
                             const struct token_arr* res,
                             size_t macro_start,
                             struct code_source* src) {
    size_t i = macro_start;
    assert(res->tokens[i].type == IDENTIFIER);
    ++i;
    assert(res->tokens[i].type == LBRACKET);
    ++i;
    
    const bool can_read_new_toks = res == &state->res;
    size_t open_bracket_count = 1;
    while (i != res->len || (can_read_new_toks && !code_source_over(src))) {
        if (can_read_new_toks) {
            while (i == res->len && !code_source_over(src)) {
                if (!read_and_tokenize_line(state, src)) {
                    return (size_t)-1;
                }
            }

            if (code_source_over(src) && i == res->len) {
                break;
            }
        }

        const struct token* curr = &res->tokens[i];
        if (curr->type == LBRACKET) {
            ++open_bracket_count;
        } else if (curr->type == RBRACKET) {
            --open_bracket_count;
            if (open_bracket_count == 0) {
                break;
            }
        }
        ++i;
    }

    if (res->tokens[i].type != RBRACKET) {
        set_preproc_err(state->err,
                        PREPROC_ERR_UNTERMINATED_MACRO,
                        res->tokens[macro_start].loc);
        return (size_t)-1;
    }
    return i;
}

struct expanded_macro_stack {
    size_t len, cap;
    const char** data;
};

static void expanded_macro_stack_push(struct expanded_macro_stack* stack,
                                      const struct preproc_macro* m) {
    if (stack->len == stack->cap) {
        grow_alloc((void**)&stack->data, &stack->cap, sizeof(void*));
    }
    stack->data[stack->len] = m->spell;
    ++stack->len;
}

static void expanded_macro_stack_pop(struct expanded_macro_stack* stack) {
    --stack->len;
}

static bool expanded_macro_stack_contains(
    const struct expanded_macro_stack* stack,
    const struct preproc_macro* to_check) {
    for (size_t i = 0; i < stack->len; ++i) {
        if (stack->data[i] == to_check->spell) {
            return true;
        }
    }
    return false;
}

static struct expanded_macro_stack expanded_macro_stack_create(void) {
    return (struct expanded_macro_stack){
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

static void expanded_macro_stack_free(struct expanded_macro_stack* stack) {
    free(stack->data);
}

static size_t expand_obj_macro(struct token_arr* res,
                               const struct preproc_macro* macro,
                               size_t macro_idx,
                               struct expanded_macro_stack* expanded);

static size_t expand_func_macro(struct preproc_state* state,
                                struct token_arr* res,
                                const struct preproc_macro* macro,
                                size_t macro_idx,
                                size_t macro_end,
                                struct code_source* src,
                                struct expanded_macro_stack* expanded);

static size_t find_and_expand_macro(struct preproc_state* state,
                                    struct token_arr* res,
                                    size_t i,
                                    struct code_source* src,
                                    struct expanded_macro_stack* expanded) {
    const struct token* curr = &res->tokens[i];
    if (curr->type != IDENTIFIER) {
        return i;
    }
    const struct preproc_macro* macro = find_preproc_macro(state,
                                                           &curr->spelling);
    if (macro == NULL || expanded_macro_stack_contains(expanded, macro)) {
        return i;
    }
    size_t macro_end;
    if (macro->is_func_macro) {
        const size_t next_idx = i + 1;
        if (next_idx < res->len && res->tokens[next_idx].type == LBRACKET) {
            macro_end = find_macro_end(state, res, i, src);
            if (state->err->type != PREPROC_ERR_NONE) {
                return (size_t)-1;
            }
            assert(macro_end != (size_t)-1);
            return expand_func_macro(state, res, macro, i, macro_end, src, expanded);
        } else {
            // not considered func_macro without brackets
            return i;
        }
    } else {
        return expand_obj_macro(res, macro, i, expanded);
    }
}

static bool expand_all_macros_internal(struct preproc_state* state,
                                       struct token_arr* res,
                                       size_t start,
                                       struct code_source* src,
                                       struct expanded_macro_stack* expanded) {
    for (size_t i = start; i < res->len; ++i) {
        const size_t next = find_and_expand_macro(state, res, i, src, expanded);
        if (next == (size_t)-1) {
            return false;
        }
        i = next;
    }

    return true;
}

bool expand_all_macros(struct preproc_state* state,
                       struct token_arr* res,
                       size_t start,
                       struct code_source* src) {

    struct expanded_macro_stack expanded = expanded_macro_stack_create();
    const bool success = expand_all_macros_internal(state,
                                                    res,
                                                    start,
                                                    src,
                                                    &expanded);
    expanded_macro_stack_free(&expanded);
    return success;
}

static size_t get_str_idx(const char** strs, size_t len, const char* to_find) {
    for (size_t i = 0; i < len; ++i) {
        if (strcmp(strs[i], to_find) == 0) {
            return i;
        }
    }
    return (size_t)-1;
}

static struct preproc_macro parse_func_like_macro(struct token_arr* arr,
                                                  const char* spell,
                                                  struct preproc_err* err) {
    struct preproc_macro res = {
        .spell = spell,
        .is_func_macro = true,
    };

    size_t it = 4;
    res.num_args = 0;

    const char** arg_spells = NULL;
    size_t arg_spells_cap = 0;
    while (it < arr->len && arr->tokens[it].type != RBRACKET
           && arr->tokens[it].type != ELLIPSIS) {
        if (arr->tokens[it].type != IDENTIFIER) {
            set_preproc_err(err,
                            PREPROC_ERR_EXPECTED_TOKENS,
                            arr->tokens[it].loc);
            const enum token_type ex[] = {
                ELLIPSIS,
                IDENTIFIER,
            };
            err->expected_tokens_err = create_expected_tokens_err(
                arr->tokens[it].type,
                ex,
                ARR_LEN(ex));
            goto fail;
        }

        if (arg_spells_cap == res.num_args) {
            grow_alloc((void**)&arg_spells,
                       &arg_spells_cap,
                       sizeof *arg_spells);
        }
        arg_spells[res.num_args] = str_get_data(&arr->tokens[it].spelling);
        assert(arg_spells[res.num_args]);

        ++res.num_args;
        ++it;

        if (arr->tokens[it].type != RBRACKET) {
            if (arr->tokens[it].type != COMMA) {
                set_preproc_err(err,
                                PREPROC_ERR_EXPECTED_TOKENS,
                                arr->tokens[it].loc);
                const enum token_type ex[] = {
                    COMMA,
                    RBRACKET,
                };
                err->expected_tokens_err = create_expected_tokens_err(
                    arr->tokens[it].type,
                    ex,
                    ARR_LEN(ex));
                goto fail;
            }
            ++it;
        }
    }

    if (it == arr->len) {
        assert(arr->tokens[3].type == LBRACKET);
        set_preproc_err(err,
                        PREPROC_ERR_UNTERMINATED_MACRO,
                        arr->tokens[3].loc);
        goto fail;
    }

    if (arr->tokens[it].type == ELLIPSIS) {
        ++it;
        res.is_variadic = true;
        if (arr->tokens[it].type != RBRACKET) {
            set_preproc_err(err,
                            PREPROC_ERR_EXPECTED_TOKENS,
                            arr->tokens[it].loc);
            err->expected_tokens_err = create_expected_token_err(
                arr->tokens[it].type,
                RBRACKET);
            goto fail;
        }
    } else {
        res.is_variadic = false;
    }

    assert(arr->tokens[it].type == RBRACKET);

    res.expansion_len = arr->len - it - 1; // TODO: not sure about - 1
    res.expansion = res.expansion_len == 0
                        ? NULL
                        : xmalloc(sizeof *res.expansion * res.expansion_len);

    for (size_t i = it + 1; i < arr->len; ++i) {
        const size_t res_idx = i - it - 1;

        struct token* curr_tok = &arr->tokens[i];
        struct token_or_arg* res_curr = &res.expansion[res_idx];
        if (curr_tok->type == IDENTIFIER) {
            if (res.is_variadic
                && strcmp(str_get_data(&curr_tok->spelling), "__VA_ARGS__")
                       == 0) {
                res_curr->is_arg = true;
                res_curr->arg_num = res.num_args;
                continue;
            }

            const size_t idx = get_str_idx(arg_spells,
                                           res.num_args,
                                           str_get_data(&curr_tok->spelling));

            if (idx != (size_t)-1) {
                res_curr->is_arg = true;
                res_curr->arg_num = idx;
                continue;
            }
        }

        res_curr->is_arg = false;
        res_curr->token = *curr_tok;
        curr_tok->spelling = create_null_str();
    }

    // cast to make msvc happy (though it shouldn't be like this)
    free((void*)arg_spells);
    return res;
fail:
    free((void*)arg_spells);
    return (struct preproc_macro){0};
}

static struct token move_token(struct token* t);

static struct preproc_macro parse_object_like_macro(struct token_arr* arr,
                                                    const char* spell) {
    struct preproc_macro res = {
        .spell = spell,
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,
        .expansion_len = arr->len - 3,
        .expansion = res.expansion_len == 0
                         ? NULL
                         : xmalloc(sizeof *res.expansion * res.expansion_len),
    };

    for (size_t i = 3; i < arr->len; ++i) {
        const size_t res_idx = i - 3;
        struct token_or_arg* res_curr = &res.expansion[res_idx];
        res_curr->is_arg = false;
        res_curr->token = move_token(&arr->tokens[i]);
    }
    return res;
}

struct preproc_macro parse_preproc_macro(struct token_arr* arr,
                                         const char* spell,
                                         struct preproc_err* err) {
    assert(arr);
    assert(arr->len >= 2);
    assert(arr->tokens[0].type == STRINGIFY_OP);
    assert(arr->tokens[1].type == IDENTIFIER);
    assert(strcmp(str_get_data(&arr->tokens[1].spelling), "define") == 0);

    if (arr->len < 3) {
        assert(arr->len > 0);
        set_preproc_err(err, PREPROC_ERR_EMPTY_DEFINE, arr->tokens[0].loc);
        return (struct preproc_macro){0};
    } else if (arr->tokens[2].type != IDENTIFIER) {
        set_preproc_err(err, PREPROC_ERR_DEFINE_NOT_ID, arr->tokens[2].loc);
        err->type_instead_of_identifier = arr->tokens[2].type;
        return (struct preproc_macro){0};
    }

    if (arr->len > 3 && arr->tokens[3].type == LBRACKET) {
        return parse_func_like_macro(arr, spell, err);
    } else {
        return parse_object_like_macro(arr, spell);
    }
}

void free_preproc_macro(struct preproc_macro* m) {
    for (size_t i = 0; i < m->expansion_len; ++i) {
        if (!m->expansion[i].is_arg) {
            free_token(&m->expansion[i].token);
        }
    }
    free(m->expansion);
}

static struct token copy_token(const struct token* t);

static struct token move_token(struct token* t) {
    struct token res = *t;
    t->spelling = create_null_str();
    return res;
}

static struct token_arr collect_until(struct token* start,
                                      const struct token* end) {
    const size_t len = end - start;
    struct token_arr res = {
        .len = len,
        .tokens = len == 0 ? NULL : xmalloc(sizeof *res.tokens * len),
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
    free(args->arrs);
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
        .arrs = cap == 0 ? NULL : xmalloc(sizeof *res.arrs * cap),
    };

    struct token* it = args_start + 1;
    while (res.len < expected_args && it != limit_ptr) {
        res.arrs[res.len] = collect_macro_arg(it, limit_ptr);
        it += res.arrs[res.len].len;

        ++res.len;
        if (it->type == COMMA) {
            if ((it + 1 == limit_ptr && res.len < expected_args)
                || (is_variadic && res.len == expected_args)) {
                break;
            } else {
                free_token(it);
                ++it;
            }
        }
    }

    if (it->type == COMMA
        && ((it + 1 == limit_ptr && res.len < expected_args) || is_variadic)) {
        free_token(it);
        ++it;
        res.arrs[res.len] = collect_until(it, limit_ptr);
        it += res.arrs[res.len].len;
        assert(it == limit_ptr);
        ++res.len;
    } else if (is_variadic && res.len == expected_args) {
        assert(it == limit_ptr || it == args_start + 1);
        res.arrs[res.len] = collect_until(it, limit_ptr);
        it += res.arrs[res.len].len;
        ++res.len;
    }

    if (res.len < expected_args) {
        set_preproc_err(err, PREPROC_ERR_MACRO_ARG_COUNT, it->loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = true;
        goto fail;
    } else if (it != limit_ptr) {
        set_preproc_err(err, PREPROC_ERR_MACRO_ARG_COUNT, it->loc);
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

static void shift_back(struct token* tokens,
                       size_t num,
                       size_t from,
                       size_t to) {
    memmove(tokens + from + num, tokens + from, sizeof *tokens * (to - from));
}

static void shift_forward(struct token* tokens,
                          size_t num,
                          size_t from,
                          size_t to) {
    memmove(tokens + from,
            tokens + from + num,
            sizeof *tokens * (to - from - num));
}

static void copy_into_tokens(struct token* tokens,
                             size_t* token_idx,
                             const struct token_arr* arr) {
    for (size_t i = 0; i < arr->len; ++i) {
        tokens[*token_idx] = copy_token(&arr->tokens[i]);
        ++*token_idx;
    }
}

// TODO: stringification and concatenation
static size_t expand_func_macro(struct preproc_state* state,
                                struct token_arr* res,
                                const struct preproc_macro* macro,
                                size_t macro_idx,
                                size_t macro_end,
                                struct code_source* src,
                                struct expanded_macro_stack* expanded) {
    assert(macro->is_func_macro);
    assert(macro_end < res->len);
    assert(res->tokens[macro_idx + 1].type == LBRACKET);
    struct macro_args args = collect_macro_args(res->tokens + macro_idx + 1,
                                                &res->tokens[macro_end],
                                                macro->num_args,
                                                macro->is_variadic,
                                                state->err);
    if (args.len == 0 && macro->num_args != 0) {
        assert(state->err->type != PREPROC_ERR_NONE);
        return (size_t)-1;
    }

    for (size_t i = 0; i < args.len; ++i) {
        if (!expand_all_macros_internal(state,
                                        &args.arrs[i],
                                        0,
                                        src,
                                        expanded)) {
            free_macro_args(&args);
            return false;
        }
    }

    assert((macro->is_variadic && args.len == macro->num_args + 1)
           || (!macro->is_variadic && args.len == macro->num_args));

    const size_t exp_len = get_expansion_len(macro, &args);
    const size_t macro_call_len = macro_end - macro_idx + 1;

    const bool alloc_grows = exp_len > macro_call_len;
    const size_t alloc_change = alloc_grows ? exp_len - macro_call_len
                                            : macro_call_len - exp_len;

    const size_t old_len = res->len;

    free_token(&res->tokens[macro_idx]);                      // identifier
    free_token(&res->tokens[macro_idx + 1]);                  // opening bracket
    free_token(&res->tokens[macro_idx + macro_call_len - 1]); // closing bracket

    if (alloc_grows) {
        res->len += alloc_change;
        res->cap += alloc_change;
        res->tokens = xrealloc(res->tokens, sizeof *res->tokens * res->cap);

        shift_back(res->tokens,
                   alloc_change,
                   macro_idx + macro_call_len,
                   old_len);
    } else if (alloc_change != 0) {
        res->len -= alloc_change;

        shift_forward(res->tokens, alloc_change, macro_idx + exp_len, old_len);
    }

    size_t token_idx = macro_idx;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const struct token_or_arg* curr = &macro->expansion[i];
        if (curr->is_arg) {
            copy_into_tokens(res->tokens,
                             &token_idx,
                             &args.arrs[curr->arg_num]);
        } else {
            res->tokens[token_idx] = copy_token(&curr->token);
            ++token_idx;
        }
    }

    expanded_macro_stack_push(expanded, macro);
    // TODO: expand macros
    expanded_macro_stack_pop(expanded);

    free_macro_args(&args);
    return token_idx == 0 ? 0 : token_idx - 1;
}

static size_t expand_obj_macro(struct token_arr* res,
                               const struct preproc_macro* macro,
                               size_t macro_idx,
                               struct expanded_macro_stack* expanded) {
    assert(macro->is_func_macro == false);
    assert(macro->num_args == 0);

    const size_t exp_len = macro->expansion_len;
    const size_t old_len = res->len;

    free_token(&res->tokens[macro_idx]);

    if (exp_len > 0) {
        res->cap += exp_len - 1;
        res->len += exp_len - 1;
        res->tokens = xrealloc(res->tokens, sizeof *res->tokens * res->cap);

        shift_back(res->tokens, exp_len - 1, macro_idx, old_len);
    } else {
        res->len -= 1;

        shift_forward(res->tokens, 1, macro_idx, old_len);
    }

    for (size_t i = 0; i < exp_len; ++i) {
        const struct token_or_arg* curr = &macro->expansion[i];
        assert(!curr->is_arg);

        res->tokens[macro_idx + i] = copy_token(&curr->token);
    }

    expanded_macro_stack_push(expanded, macro);
    // TODO: expand macros on expansion
    expanded_macro_stack_pop(expanded);

    size_t one_past_last_token = macro_idx + exp_len;
    if (one_past_last_token == 0) {
        return 0;
    } else {
        return one_past_last_token - 1;
    }
}

static struct token copy_token(const struct token* t) {
    assert(t);
    return (struct token){
        .type = t->type,
        .spelling = str_copy(&t->spelling),
        // TODO: identify as token from macro expansion
        .loc =
            {
                .file_idx = t->loc.file_idx,
                .file_loc = t->loc.file_loc,
            },
    };
}

