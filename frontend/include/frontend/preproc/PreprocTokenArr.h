#ifndef MYCC_FRONEND_PREPROC_PREPROC_TOKEN_ARR_H
#define MYCC_FRONEND_PREPROC_PREPROC_TOKEN_ARR_H

#include "util/StrBuf.h"
#include "util/IndexedStringSet.h"

#include "frontend/Token.h"

enum {PREPROC_TOKEN_ARR_INITIAL_ID_COUNT = TOKEN_NUM_KEYWORDS};

// val_indices is an index into the corresponding list in PreprocTokenValList
// This is separated, as we can have multiple PreprocTokenArrs (when handling
// preprocessor directives) but only ever one PreprocTokenValList
typedef struct PreprocTokenArr {
    uint32_t len, cap;
    uint8_t* kinds;
    uint32_t* val_indices;
    SourceLoc* locs;
} PreprocTokenArr;

typedef struct PreprocTokenValList {
    IndexedStringSet identifiers;
    IndexedStringSet int_consts;
    IndexedStringSet float_consts;
    IndexedStringSet str_lits;
} PreprocTokenValList;

PreprocTokenValList PreprocTokenValList_create(void);

void PreprocTokenValList_free(const PreprocTokenValList* vals);

PreprocTokenArr PreprocTokenArr_create_empty(void);

void PreprocTokenArr_free(const PreprocTokenArr* arr);

uint32_t PreprocTokenValList_add_identifier(PreprocTokenValList* vals, Str str);
uint32_t PreprocTokenValList_add_int_const(PreprocTokenValList* vals, Str str);
uint32_t PreprocTokenValList_add_float_const(PreprocTokenValList* vals, Str str);
uint32_t PreprocTokenValList_add_str_lit(PreprocTokenValList* vals, Str str);

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

void PreprocTokenValList_insert_initial_strings(PreprocTokenValList* res,
                                                const PreprocInitialStrings* initial_strs);

#endif

#endif
