#ifndef MYCC_FRONTEND_PREPROC_PREPROC_MACRO_H
#define MYCC_FRONTEND_PREPROC_PREPROC_MACRO_H

#include <stddef.h>
#include <stdbool.h>

#include "frontend/Token.h"

#include "PreprocState.h"

typedef struct {
    bool is_arg;
    union {
        size_t arg_num;
        Token token;
    };
} TokenOrArg;

typedef struct PreprocMacro {
    bool is_func_macro;
    bool is_variadic;
    size_t num_args;

    size_t expansion_len;
    TokenOrArg* expansion;
} PreprocMacro;

bool expand_all_macros(PreprocState* state, TokenArr* res, size_t start, const ArchTypeInfo* info);

PreprocMacro parse_preproc_macro(TokenArr* arr, PreprocErr* err);

void PreprocMacro_free(PreprocMacro* m);

#endif

