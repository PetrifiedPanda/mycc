#include "frontend/preproc/read_and_tokenize_line.h"

#include <ctype.h>

#include "util/macro_util.h"
#include "util/mem.h"

#include "frontend/preproc/PreprocMacro.h"

#include "tokenizer.h"

enum {
    PREPROC_LINE_BUF_LEN = 200
};

static bool is_preproc_directive(Str line) {
    size_t i = 0;
    while (i != line.len && isspace(Str_at(line, i))) {
        ++i;
    }
    
    if (i == line.len) {
        return false;
    }
    return Str_at(line, i) == '#';
}

static bool preproc_statement(PreprocState* state, TokenArr* arr);

bool read_and_tokenize_line(PreprocState* state) {
    assert(state);

    while (true) {
        if (state->line_info.next.data == NULL || *state->line_info.next.data == '\0') {
            PreprocState_read_line(state);
        }
        if (PreprocState_over(state)) {
            return true;
        }

        if (is_preproc_directive(state->line_info.next)) {
            TokenArr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };

            const bool res = tokenize_line(&arr, state->err, &state->line_info);
            if (!res) {
                TokenArr_free(&arr);
                return false;
            }

            const bool stat_res = preproc_statement(state, &arr);
            TokenArr_free(&arr);
            if (!stat_res) {
                return false;
            }
        } else {
            const bool res = tokenize_line(&state->res,
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

static bool is_cond_directive(Str line) {
    size_t i = 0;
    while (i != line.len && isspace(Str_at(line, i))) {
        ++i;
    }

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
    } else if (Str_eq(Str_substr(rest, 0, else_dir.len), else_dir) ||
               Str_eq(Str_substr(rest, 0, elif_dir.len), elif_dir) ||
               Str_eq(Str_substr(rest, 0, endif_dir.len), endif_dir)) {
        return true;
    } else {
        return false;
    }
}

// TODO: could probably be optimized
static bool skip_until_next_cond(PreprocState* state) {
    while (!PreprocState_over(state)) {
        PreprocState_read_line(state);
        if (is_cond_directive(state->line_info.next)) {
            TokenArr arr = {
                .len = 0,
                .cap = 0,
                .tokens = NULL,
            };
            const bool res = tokenize_line(&arr, state->err, &state->line_info);
            if (!res) {
                return false;
            }

            const bool stat_res = preproc_statement(state, &arr);
            TokenArr_free(&arr);
            return stat_res;
        }
    }

    PreprocErr_set(state->err,
                    PREPROC_ERR_UNTERMINATED_COND,
                    state->line_info.curr_loc);
    state->err->unterminated_cond_loc = state->conds[state->conds_len - 1].loc;
    return false;
}

static bool handle_preproc_if(PreprocState* state, bool cond, SourceLoc loc) {
    push_preproc_cond(state, loc, cond);

    if (!cond) {
        return skip_until_next_cond(state);
    }

    return true;
}

static bool handle_ifdef_ifndef(PreprocState* state, TokenArr* arr, bool is_ifndef) {
    assert(arr);
    assert(arr->tokens[0].kind == TOKEN_PP_STRINGIFY);
    assert(
        (!is_ifndef
         && Str_eq(StrBuf_as_str(&arr->tokens[1].spelling), STR_LIT("ifdef")))
        || (is_ifndef
            && Str_eq(StrBuf_as_str(&arr->tokens[1].spelling), STR_LIT("ifndef"))));
    const SourceLoc loc = arr->tokens[0].loc;

    if (arr->len < 3) {
        PreprocErr_set(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[1].loc);
        state->err->count_empty = true;
        state->err->count_dir_kind = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    } else if (arr->len > 3) {
        PreprocErr_set(state->err, PREPROC_ERR_ARG_COUNT, arr->tokens[3].loc);
        state->err->count_empty = false;
        state->err->count_dir_kind = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                               : SINGLE_MACRO_OP_IFDEF;
        return false;
    }
    if (arr->tokens[2].kind != TOKEN_IDENTIFIER) {
        PreprocErr_set(state->err,
                        PREPROC_ERR_IFDEF_NOT_ID,
                        arr->tokens[2].loc);
        state->err->not_identifier_got = arr->tokens[2].kind;
        state->err->not_identifier_op = is_ifndef ? SINGLE_MACRO_OP_IFNDEF
                                                  : SINGLE_MACRO_OP_IFDEF;
        return false;
    }

    const StrBuf* macro_spell = &arr->tokens[2].spelling;
    assert(macro_spell);
    assert(StrBuf_valid(macro_spell));
    const PreprocMacro* macro = find_preproc_macro(state, macro_spell);

    const bool cond = is_ifndef ? macro == NULL : macro != NULL;
    return handle_preproc_if(state, cond, loc);
}

static bool handle_else_elif(PreprocState* state, TokenArr* arr, bool is_else) {
    assert(arr->len > 1);
    if (state->conds_len == 0) {
        PreprocErr_set(state->err, PREPROC_ERR_MISSING_IF, arr->tokens[1].loc);
        state->err->missing_if_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        return false;
    }

    PreprocCond* curr_if = peek_preproc_cond(state);
    if (curr_if->had_else) {
        PreprocErr_set(state->err,
                        PREPROC_ERR_ELIF_ELSE_AFTER_ELSE,
                        arr->tokens[1].loc);
        state->err->elif_after_else_op = is_else ? ELSE_OP_ELSE : ELSE_OP_ELIF;
        state->err->prev_else_loc = curr_if->loc;
        return false;
    } else if (curr_if->had_true_branch) {
        return skip_until_next_cond(state);
    } else if (is_else) {
        curr_if->had_else = true;
        // TODO: just continue
    } else {
        // TODO: evaluate condition
        (void)arr;
    }

    return false;
}

static bool handle_include(PreprocState* state, TokenArr* arr) {
    assert(arr->len >= 2);
    if (arr->len == 2 || arr->len > 3) {
        PreprocErr_set(state->err,
                        PREPROC_ERR_INCLUDE_NUM_ARGS,
                        arr->tokens[1].loc);
        return false;
    }

    if (arr->tokens[2].kind != TOKEN_STRING_LITERAL) {
        if (!expand_all_macros(state, arr, 2)) {
            return false;
        }
    }

    // TODO: "<" ">" string literals
    if (arr->tokens[2].kind == TOKEN_STRING_LITERAL) {
        StrLit filename = convert_to_str_lit(&arr->tokens[2].spelling);
        if (filename.kind != STR_LIT_DEFAULT) {
            PreprocErr_set(state->err,
                            PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
                            arr->tokens[2].loc);
            return false;
        }
        if (!PreprocState_open_file(state,
                                     &filename.contents,
                                     arr->tokens[2].loc)) {
            return false;
        }
        return true;
    } else {
        PreprocErr_set(state->err,
                        PREPROC_ERR_INCLUDE_NOT_STRING_LITERAL,
                        arr->tokens[2].loc);
        return false;
    }
}

static bool preproc_statement(PreprocState* state, TokenArr* arr) {
    assert(arr);
    assert(arr->tokens);
    assert(arr->tokens[0].kind == TOKEN_PP_STRINGIFY);
    if (arr->len == 1) {
        return true;
    } else if (arr->tokens[1].kind != TOKEN_IDENTIFIER) {
        PreprocErr_set(state->err,
                        PREPROC_ERR_INVALID_PREPROC_DIR,
                        arr->tokens[1].loc);
        return false;
    }

    const Str directive = StrBuf_as_str(&arr->tokens[1].spelling);
    assert(Str_valid(directive));

    if (Str_eq(directive, STR_LIT("if"))) {
        // TODO:
    } else if (Str_eq(directive, STR_LIT("ifdef"))) {
        return handle_ifdef_ifndef(state, arr, false);
    } else if (Str_eq(directive, STR_LIT("ifndef"))) {
        return handle_ifdef_ifndef(state, arr, true);
    } else if (Str_eq(directive, STR_LIT("define"))) {
        const StrBuf spell = Token_take_spelling(&arr->tokens[2]);
        PreprocMacro macro = parse_preproc_macro(arr, state->err);
        if (state->err->kind != PREPROC_ERR_NONE) {
            return false;
        }
        register_preproc_macro(state, &spell, &macro);
    } else if (Str_eq(directive, STR_LIT("undef"))) {
        if (arr->len < 3) {
            PreprocErr_set(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[1].loc);
            state->err->count_empty = true;
            state->err->count_dir_kind = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->len > 3) {
            PreprocErr_set(state->err,
                            PREPROC_ERR_ARG_COUNT,
                            arr->tokens[3].loc);
            state->err->count_empty = false;
            state->err->count_dir_kind = SINGLE_MACRO_OP_UNDEF;
            return false;
        } else if (arr->tokens[2].kind != TOKEN_IDENTIFIER) {
            PreprocErr_set(state->err,
                            PREPROC_ERR_IFDEF_NOT_ID,
                            arr->tokens[2].loc);
            state->err->not_identifier_got = arr->tokens[2].kind;
            state->err->not_identifier_op = SINGLE_MACRO_OP_UNDEF;
            return false;
        }

        remove_preproc_macro(state, &arr->tokens[2].spelling);
    } else if (Str_eq(directive, STR_LIT("include"))) {
        return handle_include(state, arr);
    } else if (Str_eq(directive, STR_LIT("pragma"))) {
        // TODO:
    } else if (Str_eq(directive, STR_LIT("elif"))) {
        return handle_else_elif(state, arr, false);
    } else if (Str_eq(directive, STR_LIT("else"))) {
        return handle_else_elif(state, arr, true);
    } else if (Str_eq(directive, STR_LIT("endif"))) {
        if (state->conds_len == 0) {
            PreprocErr_set(state->err,
                            PREPROC_ERR_MISSING_IF,
                            arr->tokens[1].loc);
            state->err->missing_if_op = ELSE_OP_ENDIF;
            return false;
        }

        pop_preproc_cond(state);
    } else {
        PreprocErr_set(state->err,
                        PREPROC_ERR_INVALID_PREPROC_DIR,
                        arr->tokens[1].loc);
        return false;
    }

    return true;
}

