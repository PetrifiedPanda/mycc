#ifndef MYCC_FRONEND_PREPROC_PREPROC_TOKEN_ARR_H
#define MYCC_FRONEND_PREPROC_PREPROC_TOKEN_ARR_H

#include "util/IndexedStringSet.h"
#include "util/macro_util.h"

#include "frontend/Token.h"

enum {
    // If and else are already keywords, so they do not need to be inserted
    PREPROC_IF_ID_IDX = TOKEN_IF - TOKEN_KEYWORDS_START,
    PREPROC_ELSE_ID_IDX = TOKEN_ELSE - TOKEN_KEYWORDS_START,

    PREPROC_IFDEF_ID_IDX = TOKEN_NUM_KEYWORDS + 0,
    PREPROC_IFNDEF_ID_IDX = TOKEN_NUM_KEYWORDS + 1,
    PREPROC_DEFINE_ID_IDX = TOKEN_NUM_KEYWORDS + 2,
    PREPROC_UNDEF_ID_IDX = TOKEN_NUM_KEYWORDS + 3,
    PREPROC_INCLUDE_ID_IDX = TOKEN_NUM_KEYWORDS + 4,
    PREPROC_PRAGMA_ID_IDX = TOKEN_NUM_KEYWORDS + 5,
    PREPROC_ELIF_ID_IDX = TOKEN_NUM_KEYWORDS + 6,
    PREPROC_ENDIF_ID_IDX = TOKEN_NUM_KEYWORDS + 7,
    PREPROC_VA_ARGS_ID_IDX = TOKEN_NUM_KEYWORDS + 8,
    PREPROC_DEFINED_ID_IDX = TOKEN_NUM_KEYWORDS + 9,
};

// Because the indices are defined relative to what will be inserted before the
// preproc identifiers, they need to be offset back in the array used to add them
// Can't use STR_LIT here, because MSVC does not accept it as a constant expression
#define PLACE_OFFSET(id_idx, str) [id_idx - TOKEN_NUM_KEYWORDS] = {sizeof str - 1, str}

static const Str preproc_identifiers[] = {
    PLACE_OFFSET(PREPROC_IFDEF_ID_IDX, "ifdef"),
    PLACE_OFFSET(PREPROC_IFNDEF_ID_IDX, "ifndef"),
    PLACE_OFFSET(PREPROC_DEFINE_ID_IDX, "define"),
    PLACE_OFFSET(PREPROC_UNDEF_ID_IDX, "undef"),
    PLACE_OFFSET(PREPROC_INCLUDE_ID_IDX, "include"),
    PLACE_OFFSET(PREPROC_PRAGMA_ID_IDX, "pragma"),
    PLACE_OFFSET(PREPROC_ELIF_ID_IDX, "elif"),
    PLACE_OFFSET(PREPROC_ENDIF_ID_IDX, "endif"),
    PLACE_OFFSET(PREPROC_VA_ARGS_ID_IDX, "__VA_ARGS__"),
    PLACE_OFFSET(PREPROC_DEFINED_ID_IDX, "defined"),
};

#undef PLACE_OFFSET

enum { PREPROC_TOKEN_ARR_INITIAL_ID_COUNT = TOKEN_NUM_KEYWORDS + ARR_LEN(preproc_identifiers) };

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
