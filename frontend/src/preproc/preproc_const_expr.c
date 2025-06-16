#include "frontend/preproc/preproc_const_expr.h"

#include <string.h>

#include "util/macro_util.h"
#include "util/mem.h"

#include "frontend/ArchTypeInfo.h"

#include "frontend/preproc/PreprocMacro.h"

// TODO: Target (u)intmax_t semantics
typedef struct PreprocConstExprVal {
    bool valid;
    bool is_signed;
    union {
        uint64_t uint_val;
        int64_t sint_val;
    };
} PreprocConstExprVal;

#define CHECKED_OP(res, v2, OP)                                                \
    do {                                                                       \
        if (res.is_signed) {                                                   \
            int64_t val;                                                       \
            if (v2.is_signed) {                                                \
                val = v2.sint_val;                                             \
            } else {                                                           \
                val = v2.uint_val;                                             \
                if ((uint64_t)val != v2.uint_val) {                            \
                    /* TODO: err */                                            \
                    return (PreprocConstExprVal){0};                           \
                }                                                              \
            }                                                                  \
            res.sint_val = res.sint_val OP val;                                \
        } else if (v2.is_signed) {                                             \
            const int64_t new_val = (int64_t)res.uint_val;                     \
            if ((uint64_t)new_val != res.uint_val) {                           \
                /* TODO: err*/                                                 \
                return (PreprocConstExprVal){0};                               \
            }                                                                  \
            res.is_signed = true;                                              \
            res.sint_val = new_val OP v2.sint_val;                             \
        } else {                                                               \
            res.uint_val = res.uint_val OP v2.uint_val;                        \
        }                                                                      \
    } while (0)

static bool PreprocConstExprVal_is_nonzero(const PreprocConstExprVal* val) {
    assert(val->valid);
    return val->is_signed ? val->sint_val != 0 : val->uint_val != 0;
}

static PreprocConstExprVal evaluate_preproc_cond_expr(uint32_t* it,
                                                      const TokenArr* arr,
                                                      PreprocErr* err);

static PreprocConstExprVal evaluate_preproc_primary_expr(uint32_t* it,
                                                         const TokenArr* arr,
                                                         PreprocErr* err) {
    if (*it >= arr->len) {
        PreprocErr_set(err,
                       PREPROC_ERR_INCOMPLETE_EXPR,
                       arr->locs[*it - 1]);
        return (PreprocConstExprVal){0};
    }
    switch (arr->kinds[*it]) {
        case TOKEN_I_CONSTANT: {
            PreprocConstExprVal res;
            res.valid = true;
            const uint32_t val_idx = arr->val_indices[*it];
            if (IntValKind_is_sint(arr->int_consts[val_idx].kind)) {
                res.is_signed = true;
                res.sint_val = arr->int_consts[val_idx].sint_val;
            } else {
                res.is_signed = false;
                res.uint_val = arr->int_consts[val_idx].uint_val;
            }
            ++*it;
            return res;
        }
        case TOKEN_LBRACKET:
            ++*it;
            PreprocConstExprVal res = evaluate_preproc_cond_expr(it, arr, err);
            if (!res.valid) {
                return res;
            }
            if (arr->kinds[*it] != TOKEN_RBRACKET) {
                PreprocErr_set(err,
                               PREPROC_ERR_EXPECTED_TOKENS,
                               arr->locs[*it]);
                err->expected_tokens_err =
                    ExpectedTokensErr_create_single_token(arr->kinds[*it],
                                                          TOKEN_RBRACKET);
                return (PreprocConstExprVal){0};
            }
            ++*it;
            return res;
        default:
            PreprocErr_set(err,
                           PREPROC_ERR_EXPECTED_TOKENS,
                           arr->locs[*it]);
            static const TokenKind ex[] = {
                TOKEN_I_CONSTANT,
                TOKEN_LBRACKET,
            };
            err->expected_tokens_err = ExpectedTokensErr_create(
                arr->kinds[*it],
                ex,
                ARR_LEN(ex));
            return (PreprocConstExprVal){0};
    }
}

static bool is_preproc_unary_op(TokenKind k) {
    switch (k) {
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_BNOT:
        case TOKEN_NOT:
            return true;
        default:
            return false;
    }
}

static PreprocConstExprVal evaluate_preproc_unary_expr(uint32_t* it,
                                                       const TokenArr* arr,
                                                       PreprocErr* err) {
    if (is_preproc_unary_op(arr->kinds[*it])) {
        const TokenKind op = arr->kinds[*it];
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_unary_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        switch (op) {
            case TOKEN_ADD:
                return rhs;
            case TOKEN_SUB:
                if (rhs.is_signed) {
                    rhs.sint_val = -rhs.sint_val;
                } else {
                    const int64_t uval = rhs.uint_val;
                    if ((uint64_t)uval != rhs.uint_val) {
                        // TODO: err
                        return (PreprocConstExprVal){0};
                    }
                    rhs.is_signed = true;
                    rhs.sint_val = -uval;
                }
                break;
            case TOKEN_BNOT:
                if (rhs.is_signed) {
                    rhs.sint_val = ~rhs.sint_val;
                } else {
                    rhs.uint_val = ~rhs.uint_val;
                }
                break;
            case TOKEN_NOT:
                if (rhs.is_signed) {
                    rhs.sint_val = !rhs.sint_val;
                } else {
                    rhs.uint_val = !rhs.uint_val;
                }
                break;
            default:
                UNREACHABLE();
        }

        return rhs;
    } else {
        return evaluate_preproc_primary_expr(it, arr, err);
    }
}

static PreprocConstExprVal evaluate_preproc_mul_expr(uint32_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_unary_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && TokenKind_is_mul_op(arr->kinds[*it])) {
        const TokenKind op = arr->kinds[*it];
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_unary_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        switch (op) {
            case TOKEN_ASTERISK:
                CHECKED_OP(res, rhs, *);
                break;
            case TOKEN_DIV:
                CHECKED_OP(res, rhs, /);
                break;
            case TOKEN_MOD:
                CHECKED_OP(res, rhs, %);
                break;
            default:
                UNREACHABLE();
        }
    }

    return res;
}

static PreprocConstExprVal evaluate_preproc_add_expr(uint32_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_mul_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && TokenKind_is_add_op(arr->kinds[*it])) {
        const TokenKind op = arr->kinds[*it];
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_mul_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        if (op == TOKEN_ADD) {
            CHECKED_OP(res, rhs, +);
        } else {
            CHECKED_OP(res, rhs, -);
        }
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_shift_expr(uint32_t* it,
                                                       const TokenArr* arr,
                                                       PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_add_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && TokenKind_is_shift_op(arr->kinds[*it])) {
        const TokenKind op = arr->kinds[*it];
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_add_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        if (op == TOKEN_LSHIFT) {
            CHECKED_OP(res, rhs, <<);
        } else {
            CHECKED_OP(res, rhs, >>);
        }
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_rel_expr(uint32_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_shift_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && TokenKind_is_rel_op(arr->kinds[*it])) {
        const TokenKind op = arr->kinds[*it];
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_shift_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        // TODO: is CHECKED_OP necessary here?
        switch (op) {
            case TOKEN_LE:
                CHECKED_OP(res, rhs, <=);
                break;
            case TOKEN_GE:
                CHECKED_OP(res, rhs, >=);
                break;
            case TOKEN_LT:
                CHECKED_OP(res, rhs, <);
                break;
            case TOKEN_GT:
                CHECKED_OP(res, rhs, >);
                break;
            default:
                UNREACHABLE();
        }
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_eq_expr(uint32_t* it,
                                                    const TokenArr* arr,
                                                    PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_rel_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && TokenKind_is_eq_op(arr->kinds[*it])) {
        const bool is_eq = arr->kinds[*it] == TOKEN_EQ;
        ++*it;

        PreprocConstExprVal rhs = evaluate_preproc_rel_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        // TODO: is CHECKED_OP necessary here?
        if (is_eq) {
            CHECKED_OP(res, rhs, ==);
        } else {
            CHECKED_OP(res, rhs, !=);
        }
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_and_expr(uint32_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_eq_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && arr->kinds[*it] == TOKEN_AND) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_eq_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, &);
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_xor_expr(uint32_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_and_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && arr->kinds[*it] == TOKEN_XOR) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_and_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, ^);
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_or_expr(uint32_t* it,
                                                    const TokenArr* arr,
                                                    PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_xor_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && arr->kinds[*it] == TOKEN_OR) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_xor_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, |);
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_log_and_expr(uint32_t* it,
                                                         const TokenArr* arr,
                                                         PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_or_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && arr->kinds[*it] == TOKEN_LAND) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_or_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, &&);
        if (!PreprocConstExprVal_is_nonzero(&res)) {
            return res;
        }
    }

    return res;
}

static PreprocConstExprVal evaluate_preproc_log_or_expr(uint32_t* it,
                                                        const TokenArr* arr,
                                                        PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_log_and_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (*it < arr->len && arr->kinds[*it] == TOKEN_LOR) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_log_and_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, ||);
        if (PreprocConstExprVal_is_nonzero(&res)) {
            return res;
        }
    }

    return res;
}

static PreprocConstExprVal evaluate_preproc_cond_expr(uint32_t* it,
                                                      const TokenArr* arr,
                                                      PreprocErr* err) {
    PreprocConstExprVal curr_res = evaluate_preproc_log_or_expr(it, arr, err);
    if (!curr_res.valid) {
        return curr_res;
    }

    while (*it < arr->len && arr->kinds[*it] == TOKEN_QMARK) {
        ++*it;
        PreprocConstExprVal true_val = evaluate_preproc_log_or_expr(it,
                                                                    arr,
                                                                    err);
        if (!true_val.valid) {
            return true_val;
        }
        if (arr->kinds[*it] != TOKEN_COLON) {
            PreprocErr_set(err,
                           PREPROC_ERR_EXPECTED_TOKENS,
                           arr->locs[*it]);
            err->expected_tokens_err = ExpectedTokensErr_create_single_token(
                arr->kinds[*it],
                TOKEN_COLON);
            return (PreprocConstExprVal){0};
        }
        ++*it;
        PreprocConstExprVal false_val = evaluate_preproc_log_or_expr(it,
                                                                     arr,
                                                                     err);
        if (!false_val.valid) {
            return false_val;
        }
        curr_res = PreprocConstExprVal_is_nonzero(&curr_res) ? true_val
                                                             : false_val;
    }

    return curr_res;
}

PreprocConstExprRes evaluate_preproc_const_expr(PreprocState* state,
                                                PreprocTokenArr* arr,
                                                const ArchTypeInfo* info,
                                                PreprocErr* err) {
    for (uint32_t i = 2; i < arr->len; ++i) {
        if (arr->kinds[i] == TOKEN_IDENTIFIER
            && arr->val_indices[i] == PREPROC_DEFINED_ID_IDX) {
            if (i == arr->len - 1) {
                // TODO: error
                return (PreprocConstExprRes){
                    .valid = false,
                };
            }

            uint32_t it = i + 1;
            bool has_bracket = false;
            if (arr->kinds[it] == TOKEN_LBRACKET) {
                has_bracket = true;
                ++it;
            }

            if (arr->kinds[it] != TOKEN_IDENTIFIER) {
                PreprocErr_set(err,
                               PREPROC_ERR_EXPECTED_TOKENS,
                               arr->locs[it]);
                err->expected_tokens_err =
                    ExpectedTokensErr_create_single_token(arr->kinds[it],
                                                          TOKEN_IDENTIFIER);
                return (PreprocConstExprRes){
                    .valid = false,
                };
            }

            const bool has_macro = find_preproc_macro(state,
                                                      arr->val_indices[it])
                                   != NULL;
            ++it;

            if (has_bracket && arr->kinds[it] != TOKEN_RBRACKET) {
                PreprocErr_set(err,
                               PREPROC_ERR_EXPECTED_TOKENS,
                               arr->locs[it]);
                err->expected_tokens_err =
                    ExpectedTokensErr_create_single_token(arr->kinds[it],
                                                          TOKEN_RBRACKET);
                return (PreprocConstExprRes){
                    .valid = false,
                };
            }
            if (has_bracket) {
                ++it;
            }
            const SourceLoc loc = arr->locs[i];
            // TODO: is this correct
            //for (uint32_t j = i; j < it; ++j) {
            //    StrBuf_free(&arr->vals[j].spelling);
            //}
            Str spell = has_macro ? STR_LIT("1") : STR_LIT("0");
            arr->kinds[i] = TOKEN_I_CONSTANT;
            arr->val_indices[i] = PreprocTokenValList_add_int_const(&state->vals, spell);
            arr->locs[i] = loc;
            
            const uint32_t len = arr->len - it;
            memmove(arr->kinds + i + 1, arr->kinds + it, sizeof *arr->kinds * len);
            memmove(arr->val_indices + i + 1, arr->val_indices + it, sizeof *arr->val_indices * len);
            memmove(arr->locs + i + 1, arr->locs + it, sizeof *arr->locs * len);
            arr->len -= it - i - 1;
        }
    }

    if (!expand_all_macros(state, arr, 2, info)) {
        return (PreprocConstExprRes){
            .valid = false,
        };
    }
    const uint32_t int_consts_len = IndexedStringSet_len(&state->vals.int_consts);
    TokenArr tokens = {
        .len = arr->len,
        .cap = arr->cap,
        .kinds = arr->kinds,
        .val_indices = arr->val_indices,
        .locs = arr->locs,
        .identifiers = state->vals.identifiers._data,
        .int_consts = mycc_alloc(sizeof *tokens.int_consts * int_consts_len),
        .int_consts_len = int_consts_len,
    };
    for (uint32_t i = 0; i < int_consts_len; ++i) {
        const Str spell = IndexedStringSet_get(&state->vals.int_consts, i);
        if (Str_at(spell, 0) == '\'') {
            ParseCharConstRes res = parse_char_const(spell, info);
            if (res.err.kind != CHAR_CONST_ERR_NONE) {
                PreprocErr_set(err, PREPROC_ERR_CHAR_CONST, arr->locs[i]);
                err->char_const_err = res.err;
                // TODO: do we need to take this
                err->constant_spell = spell;
                return (PreprocConstExprRes){
                    .valid = false,
                };
            }
            tokens.int_consts[i] = res.res;
        } else {
            ParseIntConstRes res = parse_int_const(spell, info);
            if (res.err.kind != INT_CONST_ERR_NONE) {
                PreprocErr_set(err, PREPROC_ERR_INT_CONST, arr->locs[i]);
                err->int_const_err = res.err;
                // TODO: do we need to take this
                err->constant_spell = spell;
                return (PreprocConstExprRes){
                    .valid = false,
                };
            }
            tokens.int_consts[i] = res.res;
        }
    }
    // TODO: need to check for F_CONSTANT and IDENTIFIERs

    uint32_t i = 2;
    PreprocConstExprVal val = evaluate_preproc_cond_expr(&i, &tokens, err);
    if (!val.valid) {
        return (PreprocConstExprRes){
            .valid = false,
        };
    }
    assert(i == arr->len);
    mycc_free(tokens.int_consts);
    return (PreprocConstExprRes){
        .valid = true,
        .res = PreprocConstExprVal_is_nonzero(&val),
    };
}
