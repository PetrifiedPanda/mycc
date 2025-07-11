#include "frontend/preproc/PreprocMacro.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "read_and_tokenize_line.h"

// TODO: make this also collect macro args?
static uint32_t find_macro_end(PreprocState* state,
                               const PreprocTokenArr* res,
                               uint32_t macro_start,
                               const ArchTypeInfo* info) {
    uint32_t i = macro_start;
    assert(res->kinds[i] == TOKEN_IDENTIFIER);
    ++i;
    assert(res->kinds[i] == TOKEN_LBRACKET);
    ++i;

    const bool can_read_new_toks = res == &state->toks;
    uint32_t open_bracket_count = 1;
    while (i != res->len || (can_read_new_toks && !PreprocState_over(state))) {
        if (can_read_new_toks) {
            while (i == res->len && !PreprocState_over(state)) {
                if (!read_and_tokenize_line(state, info)) {
                    return UINT32_MAX;
                }
            }

            if (PreprocState_over(state) && i == res->len) {
                break;
            }
        }

        const TokenKind kind = res->kinds[i];
        if (kind == TOKEN_LBRACKET) {
            ++open_bracket_count;
        } else if (kind == TOKEN_RBRACKET) {
            --open_bracket_count;
            if (open_bracket_count == 0) {
                break;
            }
        }
        ++i;
    }

    if (res->kinds[i] != TOKEN_RBRACKET) {
        PreprocErr_set(state->err,
                       PREPROC_ERR_UNTERMINATED_MACRO,
                       res->locs[macro_start]);
        return UINT32_MAX;
    }
    return i;
}

typedef struct {
    uint32_t len, cap;
    const PreprocMacro** data;
} ExpandedMacroStack;

static void ExpandedMacroStack_push(ExpandedMacroStack* stack,
                                    const PreprocMacro* m) {
    if (stack->len == stack->cap) {
        mycc_grow_alloc((void**)&stack->data, &stack->cap, sizeof(void*));
    }
    stack->data[stack->len] = m;
    ++stack->len;
}

static void ExpandedMacroStack_pop(ExpandedMacroStack* stack) {
    --stack->len;
}

static bool ExpandedMacroStack_contains(const ExpandedMacroStack* stack,
                                        const PreprocMacro* to_check) {
    for (uint32_t i = 0; i < stack->len; ++i) {
        if (stack->data[i] == to_check) {
            return true;
        }
    }
    return false;
}

static ExpandedMacroStack ExpandedMacroStack_create(void) {
    return (ExpandedMacroStack){
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

static void ExpandedMacroStack_free(ExpandedMacroStack* stack) {
    mycc_free(stack->data);
}

typedef struct {
    int32_t alloc_change;
    uint32_t next;
} ExpansionInfo;

static ExpansionInfo expand_func_macro(PreprocState* state,
                                       PreprocTokenArr* res,
                                       const PreprocMacro* macro,
                                       uint32_t macro_idx,
                                       uint32_t macro_end,
                                       ExpandedMacroStack* expanded,
                                       const ArchTypeInfo* info);

static ExpansionInfo expand_obj_macro(PreprocState* state,
                                      PreprocTokenArr* res,
                                      const PreprocMacro* macro,
                                      uint32_t macro_idx,
                                      ExpandedMacroStack* expanded,
                                      const ArchTypeInfo* info);

static ExpansionInfo find_and_expand_macro(PreprocState* state,
                                           PreprocTokenArr* res,
                                           uint32_t i,
                                           ExpandedMacroStack* expanded,
                                           const ArchTypeInfo* info) {
    const TokenKind kind = res->kinds[i];
    if (kind != TOKEN_IDENTIFIER) {
        return (ExpansionInfo){0, i + 1};
    }
    const PreprocMacro* macro = find_preproc_macro(state, res->val_indices[i]);
    if (macro == NULL || ExpandedMacroStack_contains(expanded, macro)) {
        return (ExpansionInfo){0, i + 1};
    }
    uint32_t macro_end;
    if (macro->is_func_macro) {
        const uint32_t next_idx = i + 1;
        if (next_idx < res->len && res->kinds[next_idx] == TOKEN_LBRACKET) {
            macro_end = find_macro_end(state, res, i, info);
            if (state->err->kind != PREPROC_ERR_NONE) {
                return (ExpansionInfo){0, UINT32_MAX};
            }
            assert(macro_end != UINT32_MAX);
            return expand_func_macro(state,
                                     res,
                                     macro,
                                     i,
                                     macro_end,
                                     expanded,
                                     info);
        } else {
            // not considered func_macro without brackets
            return (ExpansionInfo){0, i + 1};
        }
    } else {
        return expand_obj_macro(state, res, macro, i, expanded, info);
    }
}

static ExpansionInfo expand_all_macros_in_range(PreprocState* state,
                                                PreprocTokenArr* res,
                                                uint32_t start,
                                                uint32_t end,
                                                ExpandedMacroStack* expanded,
                                                const ArchTypeInfo* info) {
    assert(end <= res->len);
    int32_t alloc_change = 0;
    uint32_t i = start;
    while (i < end) {
        const ExpansionInfo ex_info = find_and_expand_macro(state,
                                                            res,
                                                            i,
                                                            expanded,
                                                            info);
        if (ex_info.next == UINT32_MAX) {
            return (ExpansionInfo){0, UINT32_MAX};
        }
        end += ex_info.alloc_change;
        alloc_change += ex_info.alloc_change;
        assert(end <= res->len);
        i = ex_info.next;
    }

    return (ExpansionInfo){alloc_change, end};
}

bool expand_all_macros(PreprocState* state,
                       PreprocTokenArr* res,
                       uint32_t start,
                       const ArchTypeInfo* info) {
    // TODO: Keeping this persistent would improve performance instead of
    // reallocating all the time
    ExpandedMacroStack expanded = ExpandedMacroStack_create();
    const ExpansionInfo success = expand_all_macros_in_range(state,
                                                             res,
                                                             start,
                                                             res->len,
                                                             &expanded,
                                                             info);
    ExpandedMacroStack_free(&expanded);
    return success.next != UINT32_MAX;
}

static uint32_t get_str_idx(const uint32_t* ids, uint32_t len, uint32_t to_find) {
    for (uint32_t i = 0; i < len; ++i) {
        if (ids[i] == to_find) {
            return i;
        }
    }
    return UINT32_MAX;
}

static bool is_duplicate_arg(uint32_t id_idx,
                             const SourceLoc* loc,
                             const uint32_t* arg_ids,
                             uint32_t num_args,
                             PreprocErr* err) {
    for (uint32_t i = 0; i < num_args; ++i) {
        if (arg_ids[i] == id_idx) {
            PreprocErr_set(err, PREPROC_ERR_DUPLICATE_MACRO_PARAM, *loc);
            err->duplicate_macro_arg_id_idx = id_idx;
            return true;
        }
    }
    return false;
}

static PreprocMacro parse_func_like_macro(PreprocTokenArr* arr,
                                          PreprocErr* err) {
    PreprocMacro res = {
        .is_func_macro = true,
    };

    uint32_t it = 4;
    res.num_args = 0;

    uint32_t* arg_spells = NULL;
    uint32_t arg_spells_cap = 0;
    while (it < arr->len && arr->kinds[it] != TOKEN_RBRACKET
           && arr->kinds[it] != TOKEN_ELLIPSIS) {
        if (arr->kinds[it] != TOKEN_IDENTIFIER) {
            PreprocErr_set(err, PREPROC_ERR_EXPECTED_TOKENS, arr->locs[it]);
            static const TokenKind ex[] = {
                TOKEN_ELLIPSIS,
                TOKEN_IDENTIFIER,
            };
            err->expected_tokens_err = ExpectedTokensErr_create(arr->kinds[it],
                                                                ex,
                                                                ARR_LEN(ex));
            goto fail;
        }

        if (arg_spells_cap == res.num_args) {
            mycc_grow_alloc((void**)&arg_spells,
                            &arg_spells_cap,
                            sizeof *arg_spells);
        }
        const uint32_t spelling_val_idx = arr->val_indices[it];
        arg_spells[res.num_args] = spelling_val_idx;
        if (is_duplicate_arg(spelling_val_idx,
                             &arr->locs[it],
                             arg_spells,
                             res.num_args,
                             err)) {
            goto fail;
        }

        ++res.num_args;
        ++it;

        if (arr->kinds[it] != TOKEN_RBRACKET) {
            if (arr->kinds[it] != TOKEN_COMMA) {
                PreprocErr_set(err, PREPROC_ERR_EXPECTED_TOKENS, arr->locs[it]);
                static const TokenKind ex[] = {
                    TOKEN_COMMA,
                    TOKEN_RBRACKET,
                };
                err->expected_tokens_err = ExpectedTokensErr_create(
                    arr->kinds[it],
                    ex,
                    ARR_LEN(ex));
                goto fail;
            }
            ++it;
        }
    }

    if (it == arr->len) {
        assert(arr->kinds[3] == TOKEN_LBRACKET);
        PreprocErr_set(err, PREPROC_ERR_UNTERMINATED_MACRO, arr->locs[3]);
        goto fail;
    }

    if (arr->kinds[it] == TOKEN_ELLIPSIS) {
        ++it;
        res.is_variadic = true;
        if (arr->kinds[it] != TOKEN_RBRACKET) {
            PreprocErr_set(err, PREPROC_ERR_EXPECTED_TOKENS, arr->locs[it]);
            err->expected_tokens_err = ExpectedTokensErr_create_single_token(
                arr->kinds[it],
                TOKEN_RBRACKET);
            goto fail;
        }
    } else {
        res.is_variadic = false;
    }

    assert(arr->kinds[it] == TOKEN_RBRACKET);

    res.expansion_len = arr->len - it - 1; // TODO: not sure about - 1
    res.kinds = res.expansion_len == 0
                    ? NULL
                    : mycc_alloc(sizeof *res.kinds * res.expansion_len);
    res.vals = res.expansion_len == 0
                   ? NULL
                   : mycc_alloc(sizeof *res.vals * res.expansion_len);

    for (uint32_t i = it + 1; i < arr->len; ++i) {
        const uint32_t res_idx = i - it - 1;

        const uint32_t val_idx = arr->val_indices[i];
        const TokenKind kind = arr->kinds[i];
        uint8_t* res_kind = &res.kinds[res_idx];
        TokenValOrArg* res_val = &res.vals[res_idx];
        if (kind == TOKEN_IDENTIFIER) {
            if (res.is_variadic
                && val_idx == PREPROC_VA_ARGS_ID_IDX) {
                *res_kind = TOKEN_INVALID;
                res_val->arg_num = res.num_args;
                continue;
            }

            const uint32_t idx = get_str_idx(arg_spells,
                                             res.num_args,
                                             val_idx);

            if (idx != UINT32_MAX) {
                *res_kind = TOKEN_INVALID;
                res_val->arg_num = idx;
                continue;
            }
        }

        *res_kind = kind;
        res_val->val_idx = val_idx;
    }

    // cast to make msvc happy (though it shouldn't be like this)
    mycc_free((void*)arg_spells);
    return res;
fail:
    mycc_free((void*)arg_spells);
    return (PreprocMacro){0};
}

static PreprocMacro parse_object_like_macro(PreprocTokenArr* arr) {
    const uint32_t ex_len = arr->len - 3;
    PreprocMacro res = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,
        .expansion_len = ex_len,
        .kinds = ex_len == 0 ? NULL : mycc_alloc(sizeof *res.kinds * ex_len),
        .vals = ex_len == 0 ? NULL : mycc_alloc(sizeof *res.vals * ex_len),
    };

    for (uint32_t i = 3; i < arr->len; ++i) {
        const uint32_t res_idx = i - 3;
        res.kinds[res_idx] = arr->kinds[i];
        res.vals[res_idx].val_idx = arr->val_indices[i];
    }
    return res;
}

static bool is_func_like_macro(const PreprocTokenArr* arr, uint32_t name_len) {
    assert(arr->kinds[2] == TOKEN_IDENTIFIER);
    if (arr->len <= 3 || arr->kinds[3] != TOKEN_LBRACKET) {
        return false;
    }
    const SourceLoc macro_name_loc = arr->locs[2];
    const SourceLoc bracket_loc = arr->locs[3];
    assert(macro_name_loc.file_loc.line == bracket_loc.file_loc.line);
    const uint32_t macro_name_idx = macro_name_loc.file_loc.index;
    const uint32_t bracket_idx = bracket_loc.file_loc.index;
    return bracket_idx == macro_name_idx + name_len;
}

PreprocMacro parse_preproc_macro(PreprocTokenArr* arr,
                                 uint32_t name_len, PreprocErr* err) {
    assert(arr);
    assert(arr->len >= 2);
    assert(arr->kinds[0] == TOKEN_PP_STRINGIFY);
    assert(arr->kinds[1] == TOKEN_IDENTIFIER);
    assert(arr->val_indices[1] == PREPROC_DEFINE_ID_IDX);

    if (arr->len < 3) {
        assert(arr->len > 0);
        PreprocErr_set(err, PREPROC_ERR_EMPTY_DEFINE, arr->locs[0]);
        return (PreprocMacro){0};
    } else if (arr->kinds[2] != TOKEN_IDENTIFIER) {
        PreprocErr_set(err, PREPROC_ERR_DEFINE_NOT_ID, arr->locs[2]);
        err->type_instead_of_identifier = arr->kinds[2];
        return (PreprocMacro){0};
    }

    if (is_func_like_macro(arr, name_len)) {
        return parse_func_like_macro(arr, err);
    } else {
        return parse_object_like_macro(arr);
    }
}

void PreprocMacro_free(const PreprocMacro* m) {
    mycc_free(m->kinds);
    mycc_free(m->vals);
}

static PreprocTokenArr collect_until(PreprocTokenArr* arr, uint32_t start, uint32_t end) {
    const uint32_t len = end - start;
    PreprocTokenArr res = {0};

    if (len != 0) {
        res.len = len;
        res.cap = len;
        res.kinds = mycc_alloc(sizeof *res.kinds * len);
        res.val_indices = mycc_alloc(sizeof *res.val_indices * len);
        res.locs = mycc_alloc(sizeof *res.locs * len);
    }

    for (uint32_t i = 0; i < len; ++i) {
        const uint32_t arr_it = start + i;
        res.kinds[i] = arr->kinds[arr_it];
        res.val_indices[i] = arr->val_indices[arr_it];
        res.locs[i] = arr->locs[arr_it];
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
static PreprocTokenArr collect_macro_arg(PreprocTokenArr* arr, uint32_t it, uint32_t end) {
    uint32_t num_open_brackets = 0;
    uint32_t arg_start = it;
    while (it != end
           && (arr->kinds[it] != TOKEN_COMMA || num_open_brackets != 0)) {
        if (arr->kinds[it] == TOKEN_LBRACKET) {
            ++num_open_brackets;
        } else if (arr->kinds[it] == TOKEN_RBRACKET) {
            --num_open_brackets;
        }

        ++it;
    }

    return collect_until(arr, arg_start, it);
}

typedef struct {
    uint32_t len;
    PreprocTokenArr* arrs;
} MacroArgs;

static void PreprocTokenArr_free_only_tokens(const PreprocTokenArr* arr) {
    mycc_free(arr->kinds);
    mycc_free(arr->val_indices);
    mycc_free(arr->locs);
}

static void MacroArgs_free(MacroArgs* args) {
    for (uint32_t i = 0; i < args->len; ++i) {
        PreprocTokenArr_free_only_tokens(&args->arrs[i]);
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
static MacroArgs collect_macro_args(PreprocTokenArr* arr,
                                    uint32_t args_start,
                                    uint32_t end,
                                    uint32_t expected_args,
                                    bool is_variadic,
                                    PreprocErr* err) {
    assert(arr->kinds[args_start] == TOKEN_LBRACKET);

    const uint32_t cap = is_variadic ? expected_args + 1 : expected_args;
    assert(!is_variadic || cap != 0);
    assert(cap != 0 || args_start + 1 == end);
    MacroArgs res = {
        .len = 0,
        .arrs = cap == 0 ? NULL : mycc_alloc(sizeof *res.arrs * cap),
    };

    uint32_t it = args_start + 1;
    while (res.len < expected_args && it != end) {
        res.arrs[res.len] = collect_macro_arg(arr, it, end);
        it += res.arrs[res.len].len;

        ++res.len;
        if (arr->kinds[it] == TOKEN_COMMA) {
            if ((it + 1 == end && res.len < expected_args)
                || (is_variadic && res.len == expected_args)) {
                break;
            } else {
                ++it;
            }
        }
    }

    if (arr->kinds[it] == TOKEN_COMMA
        && ((it + 1 == end && res.len < expected_args) || is_variadic)) {
        ++it;
        res.arrs[res.len] = collect_until(arr, it, end);
        it += res.arrs[res.len].len;
        assert(it == end);
        ++res.len;
    } else if (is_variadic && res.len == expected_args) {
        assert(it == end || it == args_start + 1);
        res.arrs[res.len] = collect_until(arr, it, end);
        it += res.arrs[res.len].len;
        ++res.len;
    }

    if (res.len < expected_args) {
        PreprocErr_set(err, PREPROC_ERR_MACRO_ARG_COUNT, arr->locs[it]);
        err->expected_arg_count = expected_args;
        err->is_variadic = is_variadic;
        err->too_few_args = true;
        goto fail;
    } else if (it != end) {
        PreprocErr_set(err, PREPROC_ERR_MACRO_ARG_COUNT, arr->locs[it]);
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

static uint32_t get_expansion_len(const PreprocMacro* macro,
                                  const MacroArgs* args) {
    uint32_t len = 0;
    for (uint32_t i = 0; i < macro->expansion_len; ++i) {
        if (macro->kinds[i] == TOKEN_INVALID) {
            assert(args->arrs);
            len += args->arrs[macro->vals[i].arg_num].len;
        } else {
            len += 1;
        }
    }

    return len;
}

static void shift_back(PreprocTokenArr* arr,
                       uint32_t num,
                       uint32_t from,
                       uint32_t to) {
    const uint32_t len = to - from;
    memmove(arr->kinds + from + num,
            arr->kinds + from,
            sizeof *arr->kinds * len);
    memmove(arr->val_indices + from + num, arr->val_indices + from, sizeof *arr->val_indices * len);
    memmove(arr->locs + from + num, arr->locs + from, sizeof *arr->locs * len);
}

static void shift_forward(PreprocTokenArr* arr,
                          uint32_t num,
                          uint32_t from,
                          uint32_t to) {
    const uint32_t len = to - from - num;
    memmove(arr->kinds + from,
            arr->kinds + from + num,
            sizeof *arr->kinds * len);
    memmove(arr->val_indices + from, arr->val_indices + from + num, sizeof *arr->val_indices * len);
    memmove(arr->locs + from, arr->locs + from + num, sizeof *arr->locs * len);
}

static void copy_into_tokens(PreprocTokenArr* res,
                             uint32_t* token_idx,
                             const PreprocTokenArr* arr,
                             SourceLoc loc) {
    assert(arr);
    for (uint32_t i = 0; i < arr->len; ++i) {
        res->kinds[*token_idx] = arr->kinds[i];
        res->val_indices[*token_idx] = arr->val_indices[i];
        res->locs[*token_idx] = loc;
        ++*token_idx;
    }
}

// TODO: stringification and concatenation
static ExpansionInfo expand_func_macro(PreprocState* state,
                                       PreprocTokenArr* res,
                                       const PreprocMacro* macro,
                                       uint32_t macro_idx,
                                       uint32_t macro_end,
                                       ExpandedMacroStack* expanded,
                                       const ArchTypeInfo* info) {
    const SourceLoc loc = res->locs[macro_idx];
    assert(macro->is_func_macro);
    assert(macro_end < res->len);
    assert(res->kinds[macro_idx + 1] == TOKEN_LBRACKET);
    MacroArgs args = collect_macro_args(res,
                                        macro_idx + 1,
                                        macro_end,
                                        macro->num_args,
                                        macro->is_variadic,
                                        state->err);
    if (args.len == 0 && macro->num_args != 0) {
        assert(state->err->kind != PREPROC_ERR_NONE);
        return (ExpansionInfo){0, UINT32_MAX};
    }

    for (uint32_t i = 0; i < args.len; ++i) {
        PreprocTokenArr* arg = &args.arrs[i];
        const ExpansionInfo success = expand_all_macros_in_range(
            state,
            arg,
            0, arg->len,
            expanded,
            info);
        if (success.next == UINT32_MAX) {
            MacroArgs_free(&args);
            return success;
        }
    }

    assert((macro->is_variadic && args.len == (uint32_t)macro->num_args + 1)
           || (!macro->is_variadic && args.len == (uint32_t)macro->num_args));

    const uint32_t exp_len = get_expansion_len(macro, &args);
    const uint32_t macro_call_len = macro_end - macro_idx + 1;

    const bool alloc_grows = exp_len > macro_call_len;
    const uint32_t alloc_change_abs = alloc_grows ? exp_len - macro_call_len
                                                  : macro_call_len - exp_len;

    const uint32_t old_len = res->len;

    int32_t alloc_change = 0;
    if (alloc_grows) {
        alloc_change = alloc_change_abs;
        res->len += alloc_change_abs;
        res->cap += alloc_change_abs;
        res->kinds = mycc_realloc(res->kinds, sizeof *res->kinds * res->cap);
        res->val_indices = mycc_realloc(res->val_indices, sizeof *res->val_indices * res->cap);
        res->locs = mycc_realloc(res->locs, sizeof *res->locs * res->cap);

        shift_back(res, alloc_change_abs, macro_idx + macro_call_len, old_len);
    } else if (alloc_change_abs != 0) {
        alloc_change = -(ptrdiff_t)alloc_change_abs;
        res->len -= alloc_change_abs;

        shift_forward(res, alloc_change_abs, macro_idx + exp_len, old_len);
    }

    uint32_t token_idx = macro_idx;
    for (uint32_t i = 0; i < macro->expansion_len; ++i) {
        const TokenKind kind = macro->kinds[i];
        const TokenValOrArg* curr = &macro->vals[i];
        if (kind == TOKEN_INVALID) {
            copy_into_tokens(res, &token_idx, &args.arrs[curr->arg_num], loc);
        } else {
            res->kinds[token_idx] = kind;
            res->val_indices[token_idx] = curr->val_idx;
            res->locs[token_idx] = loc;
            ++token_idx;
        }
    }

    const uint32_t last_after_macro = token_idx == 0 ? 0 : token_idx - 1;
    ExpandedMacroStack_push(expanded, macro);
    ExpansionInfo ex_info = expand_all_macros_in_range(state,
                                                       res,
                                                       macro_idx,
                                                       last_after_macro,
                                                       expanded,
                                                       info);
    ExpandedMacroStack_pop(expanded);

    MacroArgs_free(&args);
    ex_info.alloc_change += alloc_change;
    return ex_info;
}

static ExpansionInfo expand_obj_macro(PreprocState* state,
                                      PreprocTokenArr* res,
                                      const PreprocMacro* macro,
                                      uint32_t macro_idx,
                                      ExpandedMacroStack* expanded,
                                      const ArchTypeInfo* info) {
    const SourceLoc loc = res->locs[macro_idx];
    assert(macro->is_func_macro == false);
    assert(macro->num_args == 0);

    const uint32_t exp_len = macro->expansion_len;
    const uint32_t old_len = res->len;

    int32_t alloc_change;
    if (exp_len > 0) {
        alloc_change = exp_len - 1;
        res->cap += exp_len - 1;
        res->len += exp_len - 1;
        res->kinds = mycc_realloc(res->kinds, sizeof *res->kinds * res->cap);
        res->val_indices = mycc_realloc(res->val_indices, sizeof *res->val_indices * res->cap);
        res->locs = mycc_realloc(res->locs, sizeof *res->locs * res->cap);

        shift_back(res, exp_len - 1, macro_idx, old_len);
    } else {
        alloc_change = -1;
        res->len -= 1;

        shift_forward(res, 1, macro_idx, old_len);
    }

    for (uint32_t i = 0; i < exp_len; ++i) {
        assert(macro->kinds[i] != TOKEN_INVALID);

        res->kinds[macro_idx + i] = macro->kinds[i];
        res->val_indices[macro_idx + i] = macro->vals[i].val_idx;
        res->locs[macro_idx + i] = loc;
    }

    ExpandedMacroStack_push(expanded, macro);
    ExpansionInfo ex_info = expand_all_macros_in_range(state,
                                                       res,
                                                       macro_idx,
                                                       macro_idx + exp_len,
                                                       expanded,
                                                       info);
    ExpandedMacroStack_pop(expanded);
    ex_info.alloc_change += alloc_change;
    return ex_info;
}

