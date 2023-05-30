#include "frontend/preproc/PreprocMacro.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/preproc/read_and_tokenize_line.h"

// TODO: make this also collect macro args?
static size_t find_macro_end(PreprocState* state, const TokenArr* res, size_t macro_start) {
    size_t i = macro_start;
    assert(res->tokens[i].kind == TOKEN_IDENTIFIER);
    ++i;
    assert(res->tokens[i].kind == TOKEN_LBRACKET);
    ++i;

    const bool can_read_new_toks = res == &state->res;
    size_t open_bracket_count = 1;
    while (i != res->len || (can_read_new_toks && !PreprocState_over(state))) {
        if (can_read_new_toks) {
            while (i == res->len && !PreprocState_over(state)) {
                if (!read_and_tokenize_line(state)) {
                    return (size_t)-1;
                }
            }

            if (PreprocState_over(state) && i == res->len) {
                break;
            }
        }

        const Token* curr = &res->tokens[i];
        if (curr->kind == TOKEN_LBRACKET) {
            ++open_bracket_count;
        } else if (curr->kind == TOKEN_RBRACKET) {
            --open_bracket_count;
            if (open_bracket_count == 0) {
                break;
            }
        }
        ++i;
    }

    if (res->tokens[i].kind != TOKEN_RBRACKET) {
        PreprocErr_set(state->err,
                        PREPROC_ERR_UNTERMINATED_MACRO,
                        res->tokens[macro_start].loc);
        return (size_t)-1;
    }
    return i;
}

typedef struct {
    size_t len, cap;
    const PreprocMacro** data;
} ExpandedMacroStack;

static void expanded_macro_stack_push(ExpandedMacroStack* stack,
                                      const PreprocMacro* m) {
    if (stack->len == stack->cap) {
        mycc_grow_alloc((void**)&stack->data, &stack->cap, sizeof(void*));
    }
    stack->data[stack->len] = m;
    ++stack->len;
}

static void expanded_macro_stack_pop(ExpandedMacroStack* stack) {
    --stack->len;
}

static bool expanded_macro_stack_contains(const ExpandedMacroStack* stack,
                                          const PreprocMacro* to_check) {
    for (size_t i = 0; i < stack->len; ++i) {
        if (stack->data[i] == to_check) {
            return true;
        }
    }
    return false;
}

static ExpandedMacroStack expanded_macro_stack_create(void) {
    return (ExpandedMacroStack){
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

static void expanded_macro_stack_free(ExpandedMacroStack* stack) {
    mycc_free(stack->data);
}

typedef struct {
    ptrdiff_t alloc_change;
    size_t next;
} ExpansionInfo;

static ExpansionInfo expand_func_macro(PreprocState* state,
                                       TokenArr* res,
                                       const PreprocMacro* macro,
                                       size_t macro_idx,
                                       size_t macro_end,
                                       ExpandedMacroStack* expanded);

static ExpansionInfo expand_obj_macro(PreprocState* state,
                                      TokenArr* res,
                                      const PreprocMacro* macro,
                                      size_t macro_idx,
                                      ExpandedMacroStack* expanded);

static ExpansionInfo find_and_expand_macro(PreprocState* state,
                                           TokenArr* res,
                                           size_t i,
                                           ExpandedMacroStack* expanded) {
    const Token* curr = &res->tokens[i];
    if (curr->kind != TOKEN_IDENTIFIER) {
        return (ExpansionInfo){0, i + 1};
    }
    const PreprocMacro* macro = find_preproc_macro(state, &curr->spelling);
    if (macro == NULL || expanded_macro_stack_contains(expanded, macro)) {
        return (ExpansionInfo){0, i + 1};
    }
    size_t macro_end;
    if (macro->is_func_macro) {
        const size_t next_idx = i + 1;
        if (next_idx < res->len
            && res->tokens[next_idx].kind == TOKEN_LBRACKET) {
            macro_end = find_macro_end(state, res, i);
            if (state->err->kind != PREPROC_ERR_NONE) {
                return (ExpansionInfo){0, (size_t)-1};
            }
            assert(macro_end != (size_t)-1);
            return expand_func_macro(state, res, macro, i, macro_end, expanded);
        } else {
            // not considered func_macro without brackets
            return (ExpansionInfo){0, i + 1};
        }
    } else {
        return expand_obj_macro(state, res, macro, i, expanded);
    }
}

static ExpansionInfo expand_all_macros_in_range(PreprocState* state,
                                                TokenArr* res,
                                                size_t start,
                                                size_t end,
                                                ExpandedMacroStack* expanded) {
    assert(end <= res->len);
    ptrdiff_t alloc_change = 0;
    size_t i = start;
    while (i < end) {
        const ExpansionInfo ex_info = find_and_expand_macro(state,
                                                            res,
                                                            i,
                                                            expanded);
        if (ex_info.next == (size_t)-1) {
            return (ExpansionInfo){0, (size_t)-1};
        }
        end += ex_info.alloc_change;
        alloc_change += ex_info.alloc_change;
        assert(end <= res->len);
        i = ex_info.next;
    }

    return (ExpansionInfo){alloc_change, end};
}

bool expand_all_macros(PreprocState* state, TokenArr* res, size_t start) {
    ExpandedMacroStack expanded = expanded_macro_stack_create();
    const ExpansionInfo success = expand_all_macros_in_range(state,
                                                             res,
                                                             start,
                                                             res->len,
                                                             &expanded);
    expanded_macro_stack_free(&expanded);
    return success.next != (size_t)-1;
}

static size_t get_str_idx(const char** strs, size_t len, const char* to_find) {
    for (size_t i = 0; i < len; ++i) {
        if (strcmp(strs[i], to_find) == 0) {
            return i;
        }
    }
    return (size_t)-1;
}

static bool is_duplicate_arg(Token* tok,
                             const char** arg_spells,
                             size_t num_args,
                             PreprocErr* err) {
    const char* data = Str_get_data(&tok->spelling);
    for (size_t i = 0; i < num_args; ++i) {
        if (strcmp(arg_spells[i], data) == 0) {
            PreprocErr_set(err, PREPROC_ERR_DUPLICATE_MACRO_PARAM, tok->loc);
            err->duplicate_arg_name = Token_take_spelling(tok);
            return true;
        }
    }
    return false;
}

static PreprocMacro parse_func_like_macro(TokenArr* arr, PreprocErr* err) {
    PreprocMacro res = {
        .is_func_macro = true,
    };

    size_t it = 4;
    res.num_args = 0;

    const char** arg_spells = NULL;
    size_t arg_spells_cap = 0;
    while (it < arr->len && arr->tokens[it].kind != TOKEN_RBRACKET
           && arr->tokens[it].kind != TOKEN_ELLIPSIS) {
        if (arr->tokens[it].kind != TOKEN_IDENTIFIER) {
            PreprocErr_set(err,
                            PREPROC_ERR_EXPECTED_TOKENS,
                            arr->tokens[it].loc);
            static const TokenKind ex[] = {
                TOKEN_ELLIPSIS,
                TOKEN_IDENTIFIER,
            };
            err->expected_tokens_err = ExpectedTokensErr_create(
                arr->tokens[it].kind,
                ex,
                ARR_LEN(ex));
            goto fail;
        }

        if (arg_spells_cap == res.num_args) {
            mycc_grow_alloc((void**)&arg_spells,
                            &arg_spells_cap,
                            sizeof *arg_spells);
        }
        arg_spells[res.num_args] = Str_get_data(&arr->tokens[it].spelling);
        assert(arg_spells[res.num_args]);
        if (is_duplicate_arg(&arr->tokens[it], arg_spells, res.num_args, err)) {
            goto fail;
        }

        ++res.num_args;
        ++it;

        if (arr->tokens[it].kind != TOKEN_RBRACKET) {
            if (arr->tokens[it].kind != TOKEN_COMMA) {
                PreprocErr_set(err,
                                PREPROC_ERR_EXPECTED_TOKENS,
                                arr->tokens[it].loc);
                static const TokenKind ex[] = {
                    TOKEN_COMMA,
                    TOKEN_RBRACKET,
                };
                err->expected_tokens_err = ExpectedTokensErr_create(
                    arr->tokens[it].kind,
                    ex,
                    ARR_LEN(ex));
                goto fail;
            }
            ++it;
        }
    }

    if (it == arr->len) {
        assert(arr->tokens[3].kind == TOKEN_LBRACKET);
        PreprocErr_set(err,
                        PREPROC_ERR_UNTERMINATED_MACRO,
                        arr->tokens[3].loc);
        goto fail;
    }

    if (arr->tokens[it].kind == TOKEN_ELLIPSIS) {
        ++it;
        res.is_variadic = true;
        if (arr->tokens[it].kind != TOKEN_RBRACKET) {
            PreprocErr_set(err,
                            PREPROC_ERR_EXPECTED_TOKENS,
                            arr->tokens[it].loc);
            err->expected_tokens_err = ExpectedTokensErr_create_single_token(
                arr->tokens[it].kind,
                TOKEN_RBRACKET);
            goto fail;
        }
    } else {
        res.is_variadic = false;
    }

    assert(arr->tokens[it].kind == TOKEN_RBRACKET);

    res.expansion_len = arr->len - it - 1; // TODO: not sure about - 1
    res.expansion = res.expansion_len == 0
                        ? NULL
                        : mycc_alloc(sizeof *res.expansion * res.expansion_len);

    for (size_t i = it + 1; i < arr->len; ++i) {
        const size_t res_idx = i - it - 1;

        Token* curr_tok = &arr->tokens[i];
        TokenOrArg* res_curr = &res.expansion[res_idx];
        if (curr_tok->kind == TOKEN_IDENTIFIER) {
            if (res.is_variadic
                && strcmp(Str_get_data(&curr_tok->spelling), "__VA_ARGS__")
                       == 0) {
                res_curr->is_arg = true;
                res_curr->arg_num = res.num_args;
                continue;
            }

            const size_t idx = get_str_idx(arg_spells,
                                           res.num_args,
                                           Str_get_data(&curr_tok->spelling));

            if (idx != (size_t)-1) {
                res_curr->is_arg = true;
                res_curr->arg_num = idx;
                continue;
            }
        }

        res_curr->is_arg = false;
        res_curr->token = *curr_tok;
        curr_tok->spelling = Str_create_null();
    }

    // cast to make msvc happy (though it shouldn't be like this)
    mycc_free((void*)arg_spells);
    return res;
fail:
    mycc_free((void*)arg_spells);
    return (PreprocMacro){0};
}

static Token move_token(Token* t);

static PreprocMacro parse_object_like_macro(TokenArr* arr) {
    const size_t ex_len = arr->len - 3;
    PreprocMacro res = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,
        .expansion_len = ex_len,
        .expansion = ex_len == 0 ? NULL
                                 : mycc_alloc(sizeof *res.expansion * ex_len),
    };

    for (size_t i = 3; i < arr->len; ++i) {
        const size_t res_idx = i - 3;
        TokenOrArg* res_curr = &res.expansion[res_idx];
        res_curr->is_arg = false;
        res_curr->token = move_token(&arr->tokens[i]);
    }
    return res;
}

PreprocMacro parse_preproc_macro(TokenArr* arr, PreprocErr* err) {
    assert(arr);
    assert(arr->len >= 2);
    assert(arr->tokens[0].kind == TOKEN_PP_STRINGIFY);
    assert(arr->tokens[1].kind == TOKEN_IDENTIFIER);
    assert(strcmp(Str_get_data(&arr->tokens[1].spelling), "define") == 0);

    if (arr->len < 3) {
        assert(arr->len > 0);
        PreprocErr_set(err, PREPROC_ERR_EMPTY_DEFINE, arr->tokens[0].loc);
        return (PreprocMacro){0};
    } else if (arr->tokens[2].kind != TOKEN_IDENTIFIER) {
        PreprocErr_set(err, PREPROC_ERR_DEFINE_NOT_ID, arr->tokens[2].loc);
        err->type_instead_of_identifier = arr->tokens[2].kind;
        return (PreprocMacro){0};
    }

    if (arr->len > 3 && arr->tokens[3].kind == TOKEN_LBRACKET) {
        return parse_func_like_macro(arr, err);
    } else {
        return parse_object_like_macro(arr);
    }
}

static void free_preproc_token(Token* tok) {
    Str_free(&tok->spelling);
}

void PreprocMacro_free(PreprocMacro* m) {
    for (size_t i = 0; i < m->expansion_len; ++i) {
        if (!m->expansion[i].is_arg) {
            free_preproc_token(&m->expansion[i].token);
        }
    }
    mycc_free(m->expansion);
}

static Token copy_token(const Token* t);

static Token move_token(Token* t) {
    Token res = *t;
    t->spelling = Str_create_null();
    return res;
}

static TokenArr collect_until(Token* start, const Token* end) {
    const size_t len = end - start;
    TokenArr res = {
        .len = len,
        .tokens = len == 0 ? NULL : mycc_alloc(sizeof *res.tokens * len),
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
static TokenArr collect_macro_arg(Token* it, const Token* limit_ptr) {
    size_t num_open_brackets = 0;
    Token* arg_start = it;
    while (it != limit_ptr
           && (it->kind != TOKEN_COMMA || num_open_brackets != 0)) {
        if (it->kind == TOKEN_LBRACKET) {
            ++num_open_brackets;
        } else if (it->kind == TOKEN_RBRACKET) {
            --num_open_brackets;
        }

        ++it;
    }

    return collect_until(arg_start, it);
}

typedef struct {
    size_t len;
    TokenArr* arrs;
} MacroArgs;

static void MacroArgs_free(MacroArgs* args) {
    for (size_t i = 0; i < args->len; ++i) {
        TokenArr_free(&args->arrs[i]);
    }
    mycc_free(args->arrs);
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
MacroArgs collect_macro_args(Token* args_start,
                             const Token* limit_ptr,
                             size_t expected_args,
                             bool is_variadic,
                             PreprocErr* err) {
    assert(args_start->kind == TOKEN_LBRACKET);

    const size_t cap = is_variadic ? expected_args + 1 : expected_args;
    assert(!is_variadic || cap != 0);
    assert(cap != 0 || args_start + 1 == limit_ptr);
    MacroArgs res = {
        .len = 0,
        .arrs = cap == 0 ? NULL : mycc_alloc(sizeof *res.arrs * cap),
    };

    Token* it = args_start + 1;
    while (res.len < expected_args && it != limit_ptr) {
        res.arrs[res.len] = collect_macro_arg(it, limit_ptr);
        it += res.arrs[res.len].len;

        ++res.len;
        if (it->kind == TOKEN_COMMA) {
            if ((it + 1 == limit_ptr && res.len < expected_args)
                || (is_variadic && res.len == expected_args)) {
                break;
            } else {
                Token_free(it);
                ++it;
            }
        }
    }

    if (it->kind == TOKEN_COMMA
        && ((it + 1 == limit_ptr && res.len < expected_args) || is_variadic)) {
        Token_free(it);
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
        PreprocErr_set(err, PREPROC_ERR_MACRO_ARG_COUNT, it->loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = true;
        goto fail;
    } else if (it != limit_ptr) {
        PreprocErr_set(err, PREPROC_ERR_MACRO_ARG_COUNT, it->loc);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = false;
        goto fail;
    }

    return res;
fail:
    MacroArgs_free(&res);
    return (MacroArgs){
        .len = 0,
        .arrs = NULL,
    };
}

static size_t get_expansion_len(const PreprocMacro* macro,
                                const MacroArgs* args) {
    size_t len = 0;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const TokenOrArg* item = &macro->expansion[i];
        if (item->is_arg) {
            assert(args->arrs);
            len += args->arrs[item->arg_num].len;
        } else {
            len += 1;
        }
    }

    return len;
}

static void shift_back(Token* tokens, size_t num, size_t from, size_t to) {
    memmove(tokens + from + num, tokens + from, sizeof *tokens * (to - from));
}

static void shift_forward(Token* tokens, size_t num, size_t from, size_t to) {
    memmove(tokens + from,
            tokens + from + num,
            sizeof *tokens * (to - from - num));
}

static void copy_into_tokens(Token* tokens, size_t* token_idx, const TokenArr* arr) {
    assert(arr);
    for (size_t i = 0; i < arr->len; ++i) {
        tokens[*token_idx] = copy_token(&arr->tokens[i]);
        ++*token_idx;
    }
}

// TODO: stringification and concatenation
static ExpansionInfo expand_func_macro(PreprocState* state,
                                       TokenArr* res,
                                       const PreprocMacro* macro,
                                       size_t macro_idx,
                                       size_t macro_end,
                                       ExpandedMacroStack* expanded) {
    assert(macro->is_func_macro);
    assert(macro_end < res->len);
    assert(res->tokens[macro_idx + 1].kind == TOKEN_LBRACKET);
    MacroArgs args = collect_macro_args(res->tokens + macro_idx + 1,
                                        &res->tokens[macro_end],
                                        macro->num_args,
                                        macro->is_variadic,
                                        state->err);
    if (args.len == 0 && macro->num_args != 0) {
        assert(state->err->kind != PREPROC_ERR_NONE);
        return (ExpansionInfo){0, (size_t)-1};
    }

    for (size_t i = 0; i < args.len; ++i) {
        const ExpansionInfo success = expand_all_macros_in_range(
            state,
            &args.arrs[i],
            0,
            args.arrs[i].len,
            expanded);
        if (success.next == (size_t)-1) {
            MacroArgs_free(&args);
            return success;
        }
    }

    assert((macro->is_variadic && args.len == macro->num_args + 1)
           || (!macro->is_variadic && args.len == macro->num_args));

    const size_t exp_len = get_expansion_len(macro, &args);
    const size_t macro_call_len = macro_end - macro_idx + 1;

    const bool alloc_grows = exp_len > macro_call_len;
    const size_t alloc_change_abs = alloc_grows ? exp_len - macro_call_len
                                                : macro_call_len - exp_len;

    const size_t old_len = res->len;

    Token_free(&res->tokens[macro_idx]);                      // identifier
    Token_free(&res->tokens[macro_idx + 1]);                  // opening bracket
    Token_free(&res->tokens[macro_idx + macro_call_len - 1]); // closing bracket

    ptrdiff_t alloc_change = 0;
    if (alloc_grows) {
        alloc_change = alloc_change_abs;
        res->len += alloc_change_abs;
        res->cap += alloc_change_abs;
        res->tokens = mycc_realloc(res->tokens, sizeof *res->tokens * res->cap);

        shift_back(res->tokens,
                   alloc_change_abs,
                   macro_idx + macro_call_len,
                   old_len);
    } else if (alloc_change_abs != 0) {
        alloc_change = -(ptrdiff_t)alloc_change_abs;
        res->len -= alloc_change_abs;

        shift_forward(res->tokens,
                      alloc_change_abs,
                      macro_idx + exp_len,
                      old_len);
    }

    size_t token_idx = macro_idx;
    for (size_t i = 0; i < macro->expansion_len; ++i) {
        const TokenOrArg* curr = &macro->expansion[i];
        if (curr->is_arg) {
            copy_into_tokens(res->tokens,
                             &token_idx,
                             &args.arrs[curr->arg_num]);
        } else {
            res->tokens[token_idx] = copy_token(&curr->token);
            ++token_idx;
        }
    }

    const size_t last_after_macro = token_idx == 0 ? 0 : token_idx - 1;
    expanded_macro_stack_push(expanded, macro);
    ExpansionInfo ex_info = expand_all_macros_in_range(state,
                                                       res,
                                                       macro_idx,
                                                       last_after_macro,
                                                       expanded);
    expanded_macro_stack_pop(expanded);

    MacroArgs_free(&args);
    ex_info.alloc_change += alloc_change;
    return ex_info;
}

static ExpansionInfo expand_obj_macro(PreprocState* state,
                                      TokenArr* res,
                                      const PreprocMacro* macro,
                                      size_t macro_idx,
                                      ExpandedMacroStack* expanded) {
    assert(macro->is_func_macro == false);
    assert(macro->num_args == 0);

    const size_t exp_len = macro->expansion_len;
    const size_t old_len = res->len;

    Token_free(&res->tokens[macro_idx]);

    ptrdiff_t alloc_change;
    if (exp_len > 0) {
        alloc_change = exp_len - 1;
        res->cap += exp_len - 1;
        res->len += exp_len - 1;
        res->tokens = mycc_realloc(res->tokens, sizeof *res->tokens * res->cap);

        shift_back(res->tokens, exp_len - 1, macro_idx, old_len);
    } else {
        alloc_change = -1;
        res->len -= 1;

        shift_forward(res->tokens, 1, macro_idx, old_len);
    }

    for (size_t i = 0; i < exp_len; ++i) {
        const TokenOrArg* curr = &macro->expansion[i];
        assert(!curr->is_arg);

        res->tokens[macro_idx + i] = copy_token(&curr->token);
    }

    expanded_macro_stack_push(expanded, macro);
    ExpansionInfo ex_info = expand_all_macros_in_range(state,
                                                       res,
                                                       macro_idx,
                                                       macro_idx + exp_len,
                                                       expanded);
    expanded_macro_stack_pop(expanded);
    ex_info.alloc_change += alloc_change;
    return ex_info;
}

static Token copy_token(const Token* t) {
    assert(t);
    return (Token){
        .kind = t->kind,
        .spelling = Str_copy(&t->spelling),
        // TODO: identify as token from macro expansion
        .loc =
            {
                .file_idx = t->loc.file_idx,
                .file_loc = t->loc.file_loc,
            },
    };
}

