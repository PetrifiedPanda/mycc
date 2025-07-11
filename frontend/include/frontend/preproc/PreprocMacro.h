#ifndef MYCC_FRONTEND_PREPROC_PREPROC_MACRO_H
#define MYCC_FRONTEND_PREPROC_PREPROC_MACRO_H

#include <stdbool.h>

#include "PreprocState.h"

typedef union TokenValOrArg {
    uint32_t arg_num;
    uint32_t val_idx;
} TokenValOrArg;

typedef struct PreprocMacro {
    bool is_func_macro: 1;
    bool is_variadic: 1;
    uint32_t num_args: sizeof(uint32_t) * CHAR_BIT - 2;

    uint32_t expansion_len;
    // if kinds[i] is TOKEN_INVALID, current is an argument
    uint8_t* kinds;
    TokenValOrArg* vals;
} PreprocMacro;

bool expand_all_macros(PreprocState* state, PreprocTokenArr* res,
                       uint32_t start, const ArchTypeInfo* info);

// TODO: change to accept the val idx instead of the name len
PreprocMacro parse_preproc_macro(PreprocTokenArr* arr,
                                 uint32_t name_len, PreprocErr* err);

void PreprocMacro_free(const PreprocMacro* m);

#endif

