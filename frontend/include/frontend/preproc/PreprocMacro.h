#ifndef MYCC_FRONTEND_PREPROC_PREPROC_MACRO_H
#define MYCC_FRONTEND_PREPROC_PREPROC_MACRO_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/Token.h"

#include "PreprocState.h"

typedef union {
    uint32_t arg_num;
    TokenVal val;
} TokenValOrArg;

typedef struct PreprocMacro {
    bool is_func_macro;
    bool is_variadic;
    uint32_t num_args;

    uint32_t expansion_len;
    uint8_t* kinds;
    TokenValOrArg* vals;
} PreprocMacro;

bool expand_all_macros(PreprocState* state, TokenArr* res, uint32_t start, const ArchTypeInfo* info);

PreprocMacro parse_preproc_macro(TokenArr* arr, uint32_t name_len, PreprocErr* err);

void PreprocMacro_free(PreprocMacro* m);

#endif

