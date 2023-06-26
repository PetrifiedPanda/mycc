#include "frontend/preproc/preproc_const_expr.h"

#include "util/macro_util.h"

#include "frontend/preproc/PreprocMacro.h"

// TODO: Target (u)intmax_t semantics
typedef struct {
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

static PreprocConstExprVal evaluate_preproc_primary_expr(size_t* it,
                                                         const TokenArr* arr,
                                                         PreprocErr* err) {
    if (*it >= arr->len) {
        PreprocErr_set(err,
                       PREPROC_ERR_INCOMPLETE_EXPR,
                       arr->tokens[*it - 1].loc);
        return (PreprocConstExprVal){0};
    }
    if (arr->tokens[*it].kind == TOKEN_I_CONSTANT) {
        PreprocConstExprVal res;
        res.valid = true;
        if (ValueKind_is_sint(arr->tokens[*it].val.kind)) {
            res.is_signed = true;
            res.sint_val = arr->tokens[*it].val.sint_val;
        } else {
            res.is_signed = false;
            res.uint_val = arr->tokens[*it].val.uint_val;
        }
        ++*it;
        return res;
    } else {
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

static PreprocConstExprVal evaluate_preproc_unary_expr(size_t* it,
                                                       const TokenArr* arr,
                                                       PreprocErr* err) {
    if (is_preproc_unary_op(arr->tokens[*it].kind)) {
        const TokenKind op = arr->tokens[*it].kind;
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_unary_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        switch (op) {
            case TOKEN_ADD:
                return rhs;
                break;
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

static PreprocConstExprVal evaluate_preproc_mul_expr(size_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_unary_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (TokenKind_is_mul_op(arr->tokens[*it].kind)) {
        const TokenKind op = arr->tokens[*it].kind;
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

static PreprocConstExprVal evaluate_preproc_add_expr(size_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_mul_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (TokenKind_is_add_op(arr->tokens[*it].kind)) {
        const TokenKind op = arr->tokens[*it].kind;
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

static PreprocConstExprVal evaluate_preproc_shift_expr(size_t* it,
                                                       const TokenArr* arr,
                                                       PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_add_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (TokenKind_is_shift_op(arr->tokens[*it].kind)) {
        const TokenKind op = arr->tokens[*it].kind;
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

static PreprocConstExprVal evaluate_preproc_rel_expr(size_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_shift_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (TokenKind_is_rel_op(arr->tokens[*it].kind)) {
        const TokenKind op = arr->tokens[*it].kind;
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

static PreprocConstExprVal evaluate_preproc_eq_expr(size_t* it,
                                                    const TokenArr* arr,
                                                    PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_rel_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (TokenKind_is_eq_op(arr->tokens[*it].kind)) {
        const bool is_eq = arr->tokens[*it].kind == TOKEN_EQ;
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

static PreprocConstExprVal evaluate_preproc_and_expr(size_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_eq_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (arr->tokens[*it].kind == TOKEN_AND) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_eq_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, &);
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_xor_expr(size_t* it,
                                                     const TokenArr* arr,
                                                     PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_and_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (arr->tokens[*it].kind == TOKEN_XOR) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_and_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, ^);
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_or_expr(size_t* it,
                                                    const TokenArr* arr,
                                                    PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_xor_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (arr->tokens[*it].kind == TOKEN_OR) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_xor_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, |);
    }
    return res;
}

static PreprocConstExprVal evaluate_preproc_log_and_expr(size_t* it,
                                                         const TokenArr* arr,
                                                         PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_or_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (arr->tokens[*it].kind == TOKEN_LAND) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_or_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, &&);
    }

    return res;
}

static PreprocConstExprVal evaluate_preproc_log_or_expr(size_t* it,
                                                        const TokenArr* arr,
                                                        PreprocErr* err) {
    PreprocConstExprVal res = evaluate_preproc_log_and_expr(it, arr, err);
    if (!res.valid) {
        return res;
    }

    while (arr->tokens[*it].kind == TOKEN_LOR) {
        ++*it;
        PreprocConstExprVal rhs = evaluate_preproc_log_and_expr(it, arr, err);
        if (!rhs.valid) {
            return rhs;
        }

        CHECKED_OP(res, rhs, ||);
    }

    return res;
}

static PreprocConstExprVal evaluate_preproc_cond_expr(size_t* it,
                                                      const TokenArr* arr,
                                                      PreprocErr* err) {
    PreprocConstExprVal curr_res = evaluate_preproc_log_or_expr(it, arr, err);
    if (!curr_res.valid) {
        return curr_res;
    }

    while (arr->tokens[*it].kind == TOKEN_QMARK) {
        ++*it;
        PreprocConstExprVal true_val = evaluate_preproc_log_or_expr(it,
                                                                    arr,
                                                                    err);
        if (!true_val.valid) {
            return true_val;
        }
        if (arr->tokens[*it].kind != TOKEN_COLON) {
            // TODO: error
            return (PreprocConstExprVal){0};
        }
        PreprocConstExprVal false_val = evaluate_preproc_log_or_expr(it,
                                                                     arr,
                                                                     err);
        if (!false_val.valid) {
            return false_val;
        }
        curr_res = PreprocConstExprVal_is_nonzero(&curr_res) ? true_val
                                                             : false_val;
    }

    // TODO: if not over, error

    return curr_res;
}

PreprocConstExprRes evaluate_preproc_const_expr(PreprocState* state,
                                                TokenArr* arr,
                                                PreprocErr* err) {
    // TODO: don't expand macros in defined (and handle defined)
    if (!expand_all_macros(state, arr, 2)) {
        return (PreprocConstExprRes){
            .valid = false,
        };
    }

    // TODO: convert all numeric literals
    size_t i = 2;
    PreprocConstExprVal val = evaluate_preproc_cond_expr(&i, arr, err);
    if (!val.valid) {
        return (PreprocConstExprRes){
            .valid = false,
        };
    }
    return (PreprocConstExprRes){
        .valid = true,
        .res = PreprocConstExprVal_is_nonzero(&val),
    };
}
