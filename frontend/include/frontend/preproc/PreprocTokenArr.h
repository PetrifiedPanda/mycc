#ifndef MYCC_FRONEND_PREPROC_PREPROC_TOKEN_ARR_H
#define MYCC_FRONEND_PREPROC_PREPROC_TOKEN_ARR_H

#include "util/StrBuf.h"

#include "frontend/Token.h"

typedef struct PreprocTokenArr {
    uint32_t len, cap;
    uint8_t* kinds;
    uint32_t* val_indices;
    SourceLoc* locs;
    StrBuf* identifiers;
    StrBuf* int_consts;
    StrBuf* float_consts;
    StrBuf* str_lits;
    uint32_t identifiers_len;
    uint32_t int_consts_len;
    uint32_t float_consts_len;
    uint32_t str_lits_len;
} PreprocTokenArr;

PreprocTokenArr PreprocTokenArr_create_empty(void);

void PreprocTokenArr_free(const PreprocTokenArr* arr);

uint32_t PreprocTokenArr_add_identifier(PreprocTokenArr* arr, Str str);
uint32_t PreprocTokenArr_add_int_const(PreprocTokenArr* arr, Str str);
uint32_t PreprocTokenArr_add_float_const(PreprocTokenArr* arr, Str str);
uint32_t PreprocTokenArr_add_str_lit(PreprocTokenArr* arr, Str str);

#ifdef MYCC_TEST_FUNCTIONALITY


// Delivers strings to be inserted before starting parsing so we have guaranteed
// indices for testing macros
typedef struct PreprocInitialStrings {
    const Str* identifiers;
    const Str* int_consts;
    const Str* float_consts;
    const Str* str_lits;
    uint32_t identifiers_len;
    uint32_t int_consts_len;
    uint32_t float_consts_len;
    uint32_t str_lits_len;
} PreprocInitialStrings;

void PreprocTokenArr_insert_initial_strings(PreprocTokenArr* res,
                                            const PreprocInitialStrings* initial_strs);

#endif

#endif
