#include "frontend/preproc/preproc_const_expr.h"

#include "frontend/preproc/PreprocMacro.h"

typedef struct {
    bool valid;
    bool is_signed;
    union {
        uint64_t uint_val;
        int64_t sint_val;
    };
} PreprocConstExprVal;

static bool PreprocConstExprVal_is_nonzero(const PreprocConstExprVal* val) {
    assert(val->valid);
    return val->is_signed ? val->sint_val != 0 : val->uint_val != 0;
}

static PreprocConstExprVal evaluate_preproc_log_or_expr(size_t* it,
                                                        TokenArr* arr) {
    (void)it;
    (void)arr;
    // TODO:
    return (PreprocConstExprVal){0};
}

static PreprocConstExprVal evaluate_preproc_cond_expr(size_t* it,
                                                      TokenArr* arr) {
    PreprocConstExprVal curr_res = evaluate_preproc_log_or_expr(it, arr);
    if (!curr_res.valid) {
        return curr_res;
    }

    while (arr->tokens[*it].kind == TOKEN_QMARK) {
        ++*it;
        PreprocConstExprVal true_val = evaluate_preproc_log_or_expr(it, arr);
        if (!true_val.valid) {
            return true_val;
        }
        if (arr->tokens[*it].kind != TOKEN_COLON) {
            // TODO: error
            return (PreprocConstExprVal){0};
        }
        PreprocConstExprVal false_val = evaluate_preproc_log_or_expr(it, arr);
        if (!false_val.valid) {
            return false_val;
        }
        curr_res = PreprocConstExprVal_is_nonzero(&curr_res) ? true_val : false_val;
    }

    // TODO: if not over, error

    return curr_res;
}

PreprocConstExprRes evaluate_preproc_const_expr(PreprocState* state,
                                                       TokenArr* arr) {
    for (size_t i = 2; i < arr->len; ++i) {
        if (Str_eq(StrBuf_as_str(&arr->tokens[i].spelling),
                   STR_LIT("defined"))) {
            // TODO: handle defined
        }
    }
    if (!expand_all_macros(state, arr, 2)) {
        return (PreprocConstExprRes){
            .valid = false,
        };
    }

    // TODO: convert all numeric literals
    size_t i = 2;
    PreprocConstExprVal val = evaluate_preproc_cond_expr(&i, arr);
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
