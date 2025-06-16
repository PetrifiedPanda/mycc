#include "read_and_tokenize_line.h"

#include <ctype.h>

#include "frontend/preproc/PreprocMacro.h"
#include "frontend/preproc/PreprocTokenArr.h"
#include "frontend/preproc/preproc_const_expr.h"

#include "tokenizer.h"

static bool is_preproc_directive(Str line) {
    uint32_t i = 0;
    while (i != line.len && isspace(Str_at(line, i))) {
        ++i;
    }

    if (i == line.len) {
        return false;
    }
    return Str_at(line, i) == '#';
}

static bool preproc_statement(PreprocState* state,
                              PreprocTokenArr* arr,
                              const ArchTypeInfo* info);

bool read_and_tokenize_line(PreprocState* state, const ArchTypeInfo* info) {
    assert(state);

    while (true) {
        if (state->line_info.next.data == NULL
            || *state->line_info.next.data == '\0') {
            PreprocState_read_line(state);
        }
        if (PreprocState_over(state)) {
            return true;
        }

        if (is_preproc_directive(state->line_info.next)) {
            PreprocTokenArr arr = PreprocTokenArr_create_empty();

            const bool res = tokenize_line(&arr, &state->vals, state->err, &state->line_info);
            if (!res) {
                PreprocTokenArr_free(&arr);
                return false;
            }

            const bool stat_res = preproc_statement(state, &arr, info);
            PreprocTokenArr_free(&arr);
            if (!stat_res) {
                return false;
            }
        } else {
            const bool res = tokenize_line(&state->toks,
                                           &state->vals,
                                           state->err,
                                           &state->line_info);
            if (!res) {
                return false;
            }

            break;
        }
    }

    return true;
}

static uint32_t skip_whitespaces(Str line) {
    uint32_t i = 0;
    while (i != line.len && isspace(Str_at(line, i))) {
        ++i;
    }

    return i;
}

static bool is_cond_directive(Str line) {
    uint32_t i = skip_whitespaces(line);

    if (i == line.len || Str_at(line, i) != '#') {
        return false;
    }

    ++i;
    while (i != line.len && isspace(Str_at(line, i))) {
        ++i;
    }

    const Str else_dir = STR_LIT("else");
    const Str elif_dir = STR_LIT("elif");
    const Str endif_dir = STR_LIT("endif");

    Str rest = Str_advance(line, i);
    if (rest.len < else_dir.len) {
        return false;
    } else if (Str_starts_with(rest, else_dir)
               || Str_starts_with(rest, elif_dir)
               || Str_starts_with(rest, endif_dir)) {
        return true;
    } else {
        return false;
    }
}

static bool is_if_dir(Str line) {
    uint32_t i = skip_whitespaces(line);

    if (i == line.len || Str_at(line, i) != '#') {
        return false;
    }

    ++i;
    while (i != line.len && isspace(Str_at(line, i))) {
        ++i;
    }

    const Str if_dir = STR_LIT("if");
    const Str ifdef_dir = STR_LIT("ifdef");
    const Str ifndef_dir = STR_LIT("ifndef");

    Str rest = Str_advance(line, i);
    return Str_starts_with(rest, if_dir) || Str_starts_with(rest, ifdef_dir)
           || Str_starts_with(rest, ifndef_dir);
}

static bool skip_until_next_cond(PreprocState* state,
                                 const ArchTypeInfo* info) {
    while (!PreprocState_over(state)) {
        PreprocState_read_line(state);
        if (is_if_dir(state->line_info.next)) {
            PreprocState_push_cond(state, state->line_info.curr_loc, false);
            if (!skip_until_next_cond(state, info)) {
                return false;
            }
        } else if (is_cond_directive(state->line_info.next)) {
            PreprocTokenArr arr = PreprocTokenArr_create_empty();
            const bool tokenize_res = tokenize_line(&arr,
                                                    &state->vals,
                                                    state->err,
                                                    &state->line_info);
            if (!tokenize_res) {
                return false;
            }

            const bool stat_res = preproc_statement(state, &arr, info);
            PreprocTokenArr_free(&arr);
            return stat_res;
        }
    }
    PreprocErr_set(state->err,
                   PREPROC_ERR_UNTERMINATED_COND,
                   state->line_info.curr_loc);
    state->err->unterminated_cond_loc = state->conds[state->conds_len - 1].loc;
    return false;
}

static bool handle_preproc_if(PreprocState* state,
                              bool cond,
                              SourceLoc loc,
                              const ArchTypeInfo* info) {
    PreprocState_push_cond(state, loc, cond);

    if (!cond) {
        return skip_until_next_cond(state, info);
    } else {
        PreprocCond* curr = peek_preproc_cond(state);
        curr->had_true_branch = true;
        return true;
    }
}

static bool handle_ifdef_ifndef(PreprocState* state,
                                PreprocTokenArr* arr,
                                bool is_ifndef,
                                const ArchTypeInfo* info) {
    assert(arr);
    assert(arr->kinds[0] == TOKEN_PP_STRINGIFY);
    assert(arr->kinds[1] == TOKEN_IDENTIFIER);
    assert(
        (!is_ifndef
         && arr->val_indices[1] == PREPROC_IFDEF_ID_IDX)
        || (is_ifndef
            && arr->val_indices[1] == PREPROC_IFNDEF_ID_IDX));
    const SourceLoc loc = arr->locs[1];

    if (arr->len < 3) {
        PreprocErr_set(state->err, PREPROC_ERR_ARG_COUNT, arr->locs[1]);
        state->err->count_empty = true;
        state->err->count_dir_kind = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    } else if (arr->len > 3) {
        PreprocErr_set(state->err, PREPROC_ERR_ARG_COUNT, arr->locs[3]);
        state->err->count_empty = false;
        state->err->count_dir_kind = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    }
    if (arr->kinds[2] != TOKEN_IDENTIFIER) {
        PreprocErr_set(state->err,
                       PREPROC_ERR_IFDEF_NOT_ID,
                       arr->locs[2]);
        state->err->not_identifier_got = arr->kinds[2];
        state->err->not_identifier_op = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                                  : SINGLE_MACRO_OP_IFDEF;
        return false;
    }

    const PreprocMacro* macro = find_preproc_macro(state, arr->val_indices[2]);

    const bool cond = is_ifndef ? macro == NULL : macro != NULL;
    return handle_preproc_if(state, cond, loc, info);
}

static bool handle_else_elif(PreprocState* state,
                             PreprocTokenArr* arr,
                             bool is_else,
                             const ArchTypeInfo* info) {
    assert(arr->len > 1);
    if (state->conds_len == 0) {
        PreprocErr_set(state->err, PREPROC_ERR_MISSING_IF, arr->locs[1]);
        state->err->missing_if_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        return false;
    }

    PreprocCond* curr_if = peek_preproc_cond(state);
    if (curr_if->had_else) {
        PreprocErr_set(state->err,
                       PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
                       arr->locs[1]);
        state->err->elif_after_else_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        state->err->prev_else_loc = curr_if->loc;
        return false;
    } else if (curr_if->had_true_branch) {
        return skip_until_next_cond(state, info);
    } else if (is_else) {
        curr_if->had_else = true;
        // TODO: just continue
        return true;
    } else {
        PreprocConstExprRes res = evaluate_preproc_const_expr(state,
                                                              arr,
                                                              info,
                                                              state->err);
        if (!res.valid) {
            return false;
        }
        if (!res.res) {
            return skip_until_next_cond(state, info);
        }
        curr_if->had_true_branch = true;
        return true;
    }
}

static bool handle_include(PreprocState* state,
                           PreprocTokenArr* arr,
                           const ArchTypeInfo* info) {
    assert(arr->len >= 2);
    if (arr->len == 2 || arr->len > 3) {
        PreprocErr_set(state->err,
                       PREPROC_ERR_INCLUDE_NUM_ARGS,
                       arr->locs[1]);
        return false;
    }

    if (arr->kinds[2] != TOKEN_STRING_LITERAL) {
        if (!expand_all_macros(state, arr, 2, info)) {
            return false;
        }
    }

    if (arr->kinds[2] == TOKEN_STRING_LITERAL) {
        // TODO: this gets converted twice (because it will be again in convert_preproc_tokens)
        StrLit filename = convert_to_str_lit(IndexedStringSet_get(&state->vals.str_lits, arr->val_indices[2]));
        if (filename.kind != STR_LIT_DEFAULT && filename.kind != STR_LIT_INCLUDE) {
            PreprocErr_set(state->err,
                           PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
                           arr->locs[2]);
            return false;
        }
        if (!PreprocState_open_file(state,
                                    &filename.contents,
                                    &arr->locs[2])) {
            return false;
        }
        return true;
    } else {
        PreprocErr_set(state->err,
                       PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
                       arr->locs[2]);
        return false;
    }
}

static bool preproc_statement(PreprocState* state,
                              PreprocTokenArr* arr,
                              const ArchTypeInfo* info) {
    assert(arr);
    assert(arr->kinds);
    assert(arr->locs);
    assert(arr->kinds[0] == TOKEN_PP_STRINGIFY);
    if (arr->len == 1) {
        return true;
    } else if (arr->kinds[1] != TOKEN_IDENTIFIER) {
        PreprocErr_set(state->err,
                       PREPROC_ERR_INVALID_PREPROC_DIR,
                       arr->locs[1]);
        return false;
    }

    const uint32_t directive_id_idx = arr->val_indices[1];

    if (directive_id_idx == PREPROC_IF_ID_IDX) {
        PreprocConstExprRes res = evaluate_preproc_const_expr(state,
                                                              arr,
                                                              info,
                                                              state->err);
        if (!res.valid) {
            return false;
        }
        return handle_preproc_if(state, res.res, arr->locs[1], info);
    } else if (directive_id_idx == PREPROC_IFDEF_ID_IDX) {
        return handle_ifdef_ifndef(state, arr, false, info);
    } else if (directive_id_idx == PREPROC_IFNDEF_ID_IDX) {
        return handle_ifdef_ifndef(state, arr, true, info);
    } else if (directive_id_idx == PREPROC_DEFINE_ID_IDX) {
        const TokenKind macro_name_kind = arr->kinds[2];
        if (macro_name_kind != TOKEN_IDENTIFIER) {
            PreprocErr_set(state->err, PREPROC_ERR_EXPECTED_TOKENS, arr->locs[2]);
            state->err->expected_tokens_err = ExpectedTokensErr_create_single_token(macro_name_kind, TOKEN_IDENTIFIER);
            return false;
        }
        const uint32_t identifier_idx = arr->val_indices[2];
        const Str spell = IndexedStringSet_get(&state->vals.identifiers, identifier_idx);
        PreprocMacro macro = parse_preproc_macro(arr, spell.len, state->err);
        if (state->err->kind != PREPROC_ERR_NONE) {
            return false;
        }
        PreprocState_register_macro(state, identifier_idx, &macro);
    } else if (directive_id_idx == PREPROC_UNDEF_ID_IDX) {
        if (arr->len < 3) {
            PreprocErr_set(state->err,
                           PREPROC_ERR_ARG_COUNT,
                           arr->locs[1]);
            state->err->count_empty = true;
            state->err->count_dir_kind = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->len > 3) {
            PreprocErr_set(state->err,
                           PREPROC_ERR_ARG_COUNT,
                           arr->locs[3]);
            state->err->count_empty = false;
            state->err->count_dir_kind = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->kinds[2] != TOKEN_IDENTIFIER) {
            PreprocErr_set(state->err,
                           PREPROC_ERR_IFDEF_NOT_ID,
                           arr->locs[2]);
            state->err->not_identifier_got = arr->kinds[2];
            state->err->not_identifier_op = SINGLE_MACRO_OP_UNDEF;
            return false;
        }

        PreprocState_remove_macro(state, arr->val_indices[2]);
    } else if (directive_id_idx == PREPROC_INCLUDE_ID_IDX) {
        return handle_include(state, arr, info);
    } else if (directive_id_idx == PREPROC_PRAGMA_ID_IDX) {
        // TODO:
    } else if (directive_id_idx == PREPROC_ELIF_ID_IDX) {
        return handle_else_elif(state, arr, false, info);
    } else if (directive_id_idx == PREPROC_ELSE_ID_IDX) {
        return handle_else_elif(state, arr, true, info);
    } else if (directive_id_idx == PREPROC_ENDIF_ID_IDX) {
        if (state->conds_len == 0) {
            PreprocErr_set(state->err,
                           PREPROC_ERR_MISSING_IF,
                           arr->locs[1]);
            state->err->missing_if_op = ELSE_OP_ENDIF;
            return false;
        }

        PreprocState_pop_cond(state);
    } else {
        PreprocErr_set(state->err,
                       PREPROC_ERR_INVALID_PREPROC_DIR,
                       arr->locs[1]);
        return false;
    }

    return true;
}

